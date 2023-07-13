#include "core/module_manager.h"
#include "vk_renderer.h"
#include "resource_manager/resource_converter.h"
#include "engine_core/object_name.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"
#include "resource_manager/resource_manager.h"
#include "vulkan_rhi/vulkan_rhi.h"
#include "vulkan_rhi/vulkan_pipeline.h"
#include "engine_core/uuid.h"
#include "rhi/engine_rhi.h"
#include "material_asset.h"
#include "file_system/IO.h"
#include "SDL_stdinc.h"
#include "asset_loader.h"
#include "glm/common.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "mesh_asset.h"
#include "prefab_asset.h"
#include "vk_descriptors.h"
#include "vk_textures.h"
#include "vk_mesh.h"
#include "vk_renderpass.h"
#include "profiler/logger.h"

#include <limits>
#include <stdint.h>
#include <stdio.h>
#include <thread>
#include <future>
#include <unordered_map>
#include <vector>
#include "vk_types.h"
#include "vk_initializers.h"

#include <VkBootstrap.h>
#include "vk_types.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vulkan/vulkan_core.h>
#include <spirv_reflect.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <filesystem>

#include <ecs.h>
#include <core/reflection.h>
#include <engine_core/world.h>
#include "core/config_base.h"

#include "engine/vulkan_rhi_module.h"
#include "render_core/render_graph_common.h"
#include "engine_core/material/general_material_template.h"
#include "engine_core/material/shader.h"

#define VK_CHECK(x)													 \
	do																 \
	{																 \
		VkResult err = x;											 \
		if (err)													 \
		{															 \
			std::cout << "Detected Vulkan error " << err << std::endl;\
			abort();												 \
		}															 \
	} while (0)

//#define NODE_COUNT 20

using GeometryInfo = ad_astris::TransparencyFirstPassData::GeometryInfo;
using Node = ad_astris::TransparencyFirstPassData::Node;

namespace fs = std::filesystem;

std::vector<std::string> get_supported_vulkan_instance_extension()
{
	uint32_t count;
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> extensions(count);
	vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
	std::vector<std::string> result;
	for (auto& extension : extensions)
	{
		result.push_back(extension.extensionName);
	}
	
	return result;
}

std::vector<std::string> get_supported_vulkan_physical_device_extension(VkPhysicalDevice physDevice)
{
	uint32_t count;
	vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> extensions(count);
	vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &count, extensions.data());
	std::vector<std::string> result;
	for (auto& extension : extensions)
		result.push_back(extension.extensionName);
	return result;
}

// Rider says that I can't use using namespace ad_astris. Why?
namespace ad_astris
{
	void VkRenderer::init()
	{
		_sdlWindow.init({ 1700, 900 });
		auto size = _sdlWindow.get_window_size();
		_windowExtent.width = size.width;
		_windowExtent.height = size.height;

		_projectPath = fs::current_path().string();
		_projectPath.erase(_projectPath.find("\\bin"), 4);
		_fileSystem = new io::EngineFileSystem(_projectPath.c_str());

		init_vulkan();
		init_engine_systems();
		init_sync_structures();
		init_commands();
		init_swapchain();
		// Render passes should be created before framebuffers because framebuffers are created for special render passes
		init_renderpasses();
		init_framebuffers();
		init_shadow_maps();
		init_buffers();
		parse_prefabs();
		init_pipelines();
		init_output_quad();
		_userInterface.init_ui(this);

		//everything went fine
		_isInitialized = true;
	}

	uint32_t get_image_mip_levels(uint32_t width, uint32_t height);
	uint32_t previous_pow2(uint32_t v);
	
	void VkRenderer::cleanup()
	{
		_eRhi->cleanup();
		
		// I must delete objects in the reverse order of their creation
		if (_isInitialized) 
		{
			vkWaitForFences(_device, 1, &_frames[0]._renderFence, true, 1000000000);
			vkWaitForFences(_device, 1, &_frames[1]._renderFence, true, 1000000000);
			vkDeviceWaitIdle(_device);

			_userInterface.save_ini_file(this);
		
			_mainDeletionQueue.flush();
			for (int i = 0; i != FRAME_OVERLAP; ++i)
			{
				_frames[i]._frameDeletionQueue.flush();
			}

			for (int i = 0; i != _swapchainImages.size(); ++i)
			{
				vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
				vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
			}

			vkDestroyFramebuffer(_device, _transparencyFramebuffer, nullptr);
			vkDestroyFramebuffer(_device, _transparencyData.framebuffer, nullptr);
			vkDestroyFramebuffer(_device, _deferredFramebuffer, nullptr);

			_transparencyColorAttach.destroy_attachment(this);
			_transparencyDepthAttach.destroy_attachment(this);
			_transparencyVelocityAttach.destroy_attachment(this);
			_depthPyramid.destroy_texture(this);
			_transparencyData.headIndex.destroy_texture(this);
			_deferredColorAttach.destroy_attachment(this);

			for (int i = 0; i != _depthPyramidLevels; ++i)
				vkDestroyImageView(_device, _depthPyramideMips[i], nullptr);

			vkDestroySampler(_device, _depthSampler, nullptr);
			vkDestroySampler(_device, _linearSampler, nullptr);
			vkDestroySampler(_device, _nearestSampler, nullptr);
	
			vkDestroySwapchainKHR(_device, _swapchain, nullptr);

			LOG_INFO("Reset pools");
			for (int i = 0; i != FRAME_OVERLAP; ++i)
				_frames[i]._dynamicDescriptorAllocator.cleanup();
			_descriptorAllocator.cleanup();
			_descriptorLayoutCache.cleanup();
			LOG_INFO("Finish reseting pools");
			_materialSystem.cleanup();
			_renderScene.cleanup(this);
			_transparencyData.cleanup(this);
			_GBuffer.cleanup(this);
			_temporalFilter.cleanup(this);
			_composite.cleanup(this);
			vmaDestroyAllocator(_allocator);

			for (int i = 0; i != FRAME_OVERLAP; ++i)
			{
				vkDestroySemaphore(_device, _frames[i]._presentSemaphore, nullptr);
				vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);
			}
		
			vkDestroySurfaceKHR(_instance, _surface, nullptr);
			vkDestroyDevice(_device, nullptr);
			vkb::destroy_debug_utils_messenger(_instance, _debug_messenger, nullptr);
			vkDestroyInstance(_instance, nullptr);

			SDL_DestroyWindow(_sdlWindow.get_window());
		}
	}
	
	void VkRenderer::init_vulkan()
	{
		LOG_INFO("Start")

		// RHI tests
		_moduleManager = new ModuleManager();
		LOG_INFO("Loading module")
		IVulkanRHIModule* rhiModule = _moduleManager->load_module<IVulkanRHIModule>("libvulkan_rhi.dll");
		LOG_INFO("After loading module")
		_eRhi = rhiModule->create_vulkan_rhi();
		_eRhi->init(_sdlWindow.get_window(), _fileSystem);
		IVulkanDevice* vulkanDevice = _eRhi->get_device();
		_device = vulkanDevice->get_device();
		_debug_messenger = _eRhi->get_messenger();
		_surface = vulkanDevice->get_surface();
		_chosenGPU = vulkanDevice->get_physical_device();
		_instance = _eRhi->get_instance();
		_allocator = _eRhi->get_allocator();
		IVulkanQueue* queue = vulkanDevice->get_graphics_queue();
		_graphicsQueue = queue->get_queue();
		_graphicsQueueFamily = queue->get_family();

		rhi::TextureInfo info{};
		info.format = rhi::Format::R8G8B8A8_UNORM;
		info.height = 2048;
		info.width = 2048;
		info.layersCount = 6;
		info.mipLevels = 1;
		info.textureUsage = rhi::ResourceUsage::COLOR_ATTACHMENT | rhi::ResourceUsage::SAMPLED_TEXTURE;
		info.samplesCount = rhi::SampleCount::BIT_1;
		info.textureDimension = rhi::TextureDimension::TEXTURE2D;
		info.memoryUsage = rhi::MemoryUsage::GPU;
		info.resourceFlags = rhi::ResourceFlags::CUBE_TEXTURE;
		rhi::Texture texture;
		_eRhi->create_texture(&texture, &info);
		if (!texture.data)
			LOG_ERROR("ERROR")
		if (texture.data)
			LOG_INFO("Info {}", texture.size)

		rhi::TextureViewInfo viewInfo{};
		viewInfo.baseLayer = 0;
		viewInfo.baseMipLevel = 0;
		rhi::TextureView view;
		_eRhi->create_texture_view(&view, &viewInfo, &texture);
		
		rhi::RenderTarget target1;
		target1.type = rhi::RenderTargetType::COLOR;
		target1.target = &view;
		target1.initialLayout = rhi::ResourceLayout::UNDEFINED;
		target1.renderPassLayout = rhi::ResourceLayout::COLOR_ATTACHMENT;
		target1.finalLayout = rhi::ResourceLayout::COLOR_ATTACHMENT;
		target1.loadOp = rhi::LoadOp::CLEAR;
		target1.storeOp = rhi::StoreOp::STORE;

		rhi::MultiviewInfo multiviewInfo;
		multiviewInfo.isEnabled = true;
		multiviewInfo.viewCount = 6;

		rhi::RenderPassInfo passInfo;
		passInfo.renderTargets.push_back(target1);
		passInfo.pipelineType = rhi::PipelineType::GRAPHICS;
		passInfo.multiviewInfo = multiviewInfo;

		rhi::RenderPass renderPass;
		_eRhi->create_render_pass(&renderPass, &passInfo);

		rhi::SamplerInfo samplerInfo;
		samplerInfo.filter = rhi::Filter::ANISOTROPIC;
		samplerInfo.addressMode = rhi::AddressMode::CLAMP_TO_EDGE;
		samplerInfo.borderColor = rhi::BorderColor::FLOAT_OPAQUE_WHITE;
		rhi::Sampler sampler;
		_eRhi->create_sampler(&sampler, &samplerInfo);
		VkSampler* vkSampler = static_cast<VkSampler*>(sampler.handle);
		_linearSampler = *vkSampler;

		UUID uuid;

		auto renderCoreModule = _moduleManager->load_module<rcore::IRenderCoreModule>("librender_core.dll");
		LOG_INFO("Before getting shader compiler")
		_shaderCompiler = renderCoreModule->get_shader_compiler();
		LOG_INFO("Before getting render graph")
		rcore::IRenderGraph* graph = renderCoreModule->get_render_graph();
		LOG_INFO("Before creating pass")
		rcore::IRenderPass* pass = graph->add_new_pass("deferred_lighting", rcore::RenderGraphQueue::GRAPHICS);
		LOG_INFO("After creating pass")
		
		rhi::TextureInfo colorOutput;
		colorOutput.format = rhi::Format::R8G8B8A8_UNORM;
		colorOutput.samplesCount = rhi::SampleCount::BIT_1;
		colorOutput.width = 1920;
		colorOutput.height = 1080;
		colorOutput.memoryUsage = rhi::MemoryUsage::GPU;
		colorOutput.textureDimension = rhi::TextureDimension::TEXTURE2D;
		LOG_INFO("Before adding color output")
		pass->add_color_output("test_color_output", &colorOutput);
		pass->add_color_output("test_color_output2", &colorOutput);
		
		rcore::IRenderPass* thirdPass = graph->add_new_pass("composite", rcore::RenderGraphQueue::GRAPHICS);
		thirdPass->add_attachment_input("test_color_output");
		thirdPass->add_attachment_input("hdr_output");
		thirdPass->add_color_output("tonemapped", &colorOutput);
		
		rcore::IRenderPass* secondPass = graph->add_new_pass("hdr", rcore::RenderGraphQueue::GRAPHICS);
		secondPass->add_attachment_input("test_color_output2");
		secondPass->add_color_output("hdr_output", &colorOutput);

		graph->set_swap_chain_source("tonemapped");
		LOG_INFO("Before bake")
		graph->bake();
		LOG_INFO("After bake")

		graph->log();
	}

	/** Used to init material, render scene and other systems.
	Also used to get important information about hardware, etc.
	*/
	void VkRenderer::init_engine_systems()
	{
		_numThreads = std::thread::hardware_concurrency();
		_threadInfo.resize(_numThreads);

		LOG_INFO("Path is {}", _projectPath)
		LOG_INFO("Possible number of threads is {}", _numThreads)

		_descriptorAllocator.init(_device);
		_descriptorLayoutCache.init(_device);

		for (int i = 0; i != FRAME_OVERLAP; ++i)
		{
			_frames[i]._dynamicDescriptorAllocator.init(_device);
		}

		//_fileSystem = new io::EngineFileSystem(_projectPath.c_str());
		//_shaderCompiler = new rcore::ShaderCompiler(_fileSystem);
		//resource::ResourceConverter resourceConverter(_fileSystem);

		LOG_INFO("Before manager")
		//resource::ResourceManager manager(_fileSystem);
		//manager.save_resources();
		LOG_INFO("After manager")
		_shaderCompiler->init(_fileSystem);

		_materialSystem.init(this);
		_renderScene.init();
		_temporalFilter.init(this);
		_composite.init(this);

		// resource::FirstCreationContext<ecore::GeneralMaterialTemplate> materialContext;
		// materialContext.materialTemplateName = "gbuffer";
		// materialContext.vertexShaderPath = "shaders/deferred/GBuffer.vert";
		// materialContext.fragmentShaderPath = "shaders/deferred/GBuffer.frag";
		// LOG_INFO("Before first creation")
		// manager.create_new_resource(materialContext);
		// LOG_INFO("After first creation")
		// materialContext.materialTemplateName = "gbuffer2";
		// materialContext.fragmentShaderPath = "shaders/deferred/deferred_lighting.frag";
		// LOG_INFO("Before second creation")
		// manager.create_new_resource(materialContext);
		// LOG_INFO("After second creation")
		// manager.save_resources();
		// LOG_INFO("After saving resources")
		//
		// ecore::GeneralMaterialTemplateHandle handle1 = manager.get_resource<ecore::GeneralMaterialTemplate>("gbuffer");
		// ecore::GeneralMaterialTemplateHandle handle2 = manager.get_resource<ecore::GeneralMaterialTemplate>("gbuffer2");
		//
		// std::vector<ecore::GeneralMaterialTemplateHandle> handles = { handle1, handle2 };
		//
		// for (auto& handle : handles)
		// {
		// 	ecore::material::ShaderHandleContext& handleContext = handle.get_resource()->get_shader_handle_context();
		// 	std::vector<ecore::ShaderHandle> shaderHandles;
		// 	handleContext.get_all_valid_shader_handles(shaderHandles);
		// 	for (auto& shaderHandle : shaderHandles)
		// 	{
		// 		ecore::Shader* shaderObject = shaderHandle.get_resource();
		// 		ecore::shader::CompilationContext compilationContext = shaderObject->get_compilation_context();
		// 		_shaderCompiler->compile_shader_into_spv(compilationContext);
		// 		shaderObject->set_shader_compiled_flag();
		// 	}
		// }
		//
		// ecore::World* world = new ecore::World();
		//
		// io::URI levelPath = "E:/MyEngine/MyEngine/AdAstrisEngine/bin/my_level.aalevel";
		// LOG_INFO("Before creating level")
		// resource::ResourceAccessor<ecore::Level> level = manager.create_level(levelPath);
		// LOG_INFO("Before adding level")
		// world->add_level(level.get_resource());
		// LOG_INFO("Before test function")
		// world->test_function();
		// LOG_INFO("Before saving resources")
		// manager.save_resources();
		// LOG_INFO("After all manipulation with world")
		//
		// LOG_INFO("Before getting system manager")
		// ecs::SystemManager* systemManager = new ecs::SystemManager();
		// systemManager->init();
		// systemManager->add_entity_manager(world->get_entity_manager());
		// LOG_INFO("Before generating execution order")
		// systemManager->generate_execution_order();
		// LOG_INFO("Before execution")
		// systemManager->execute();
		// LOG_INFO("After execution")
		//
		// delete world;
		//delete systemManager;

		//Tests for resource converter
		 // io::URI door = "E:\\gun.gltf";
		 // io::URI texture = "D:/cyberpunk location/Texture/Gun_2/Gun_2_BaseColor.tga";
		 // io::URI gunOBJ = "E:/MyEngine/MyEngine/VulkanEngine/assets/wall.obj";
		 //
		 // auto async1 = std::async(std::launch::async, [&]()->resource::ResourceAccessor<ecore::StaticModel>{ return manager.convert_to_aares<ecore::StaticModel>(door); });
		 // auto async2 = std::async(std::launch::async, [&]()->resource::ResourceAccessor<ecore::Texture2D>{ return manager.convert_to_aares<ecore::Texture2D>(texture); });
		 // auto async3 = std::async(std::launch::async, [&]()->resource::ResourceAccessor<ecore::StaticModel>{ return manager.convert_to_aares<ecore::StaticModel>(gunOBJ); });
		 //
		 // ecore::StaticModel* model1 = async3.get().get_resource();
		 // ecore::Texture2D* texture1 = async2.get().get_resource();
		 // ecore::StaticModel* model2 = async1.get().get_resource();
		 // LOG_INFO("Model 1 name: {}", model1->get_name()->get_full_name())
		 // LOG_INFO("Model 2 name: {}", model2->get_name()->get_full_name())
		 // LOG_INFO("Texture 1 name: {}", texture1->get_name()->get_full_name())


		// io::URI aaresPath = "assets/gun.aares";
		// LOG_INFO("Before getting resource")
		// auto accessor = manager.get_resource<ecore::StaticModel>(10841365285477739537);
		// LOG_INFO("After getting resource")
		// ecore::StaticModel* model = accessor.get_resource();
		// LOG_INFO("Model name: {}", model->get_name()->get_full_name())
		// LOG_INFO("Model path: {}", model->get_path().c_str())
		// // ecore::tests();
		//
		// io::URI newTexture = "D:\\cyberpunk location\\Tile material\\metal surface\\Metal_surface_basecolor.tga";
		// //ecore::Texture2D* tempTexture = manager.convert_to_aares<ecore::Texture2D>(newTexture).get_resource();
		// ecore::Texture2D* accessor2 = manager.get_resource<ecore::Texture2D>(14124523727936834206).get_resource();
		//
		// LOG_INFO("Texture 2 name: {}", accessor2->get_name()->get_full_name())
		// LOG_INFO("Texture 2 path: {}", accessor2->get_path().c_str())
		// LOG_INFO("Texture 2 size: {}", accessor2->get_size())
		//
		// ecore::Texture2D* accessor3 = manager.get_resource<ecore::Texture2D>(10492210984281833974).get_resource();
		// LOG_INFO("Texture 3 name: {}", accessor3->get_name()->get_full_name())
		// LOG_INFO("Texture 3 path: {}", accessor3->get_path().c_str())
		// LOG_INFO("Texture 3 size: {}", accessor3->get_size())
		//
		// LOG_INFO("Before saving")
		// manager.save_resources();
		// LOG_INFO("After saving")
		// //manager.load_resource<resource::ModelInfo>(aaresPath);
		//
		//
		//
		// ecs::EntityManager system;
		// ecs::ArchetypeCreationContext creationContext;
		// ecs::Component<FirstComponent> component11; 
		//
		// io::URI path1 = "E:\\MyEngine\\MyEngine\\VulkanEngine\\bin\\level_file.txt";
	}

	void VkRenderer::init_swapchain()
	{
		LOG_INFO("Init swapchain")
		// VkSurfaceFormatKHR format;
		// format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		// format.format = VK_FORMAT_B8G8R8A8_SRGB;
		// vkb::SwapchainBuilder swapchainBuilder{_chosenGPU, _device, _surface};
		// vkb::Swapchain vkbSwapchain = swapchainBuilder
		// 	.use_default_format_selection()
		// 	.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		// 	.set_desired_extent(_windowExtent.width, _windowExtent.height)
		// 	.set_desired_format(format)
		// 	.build()
		// 	.value();
		//
		// _swapchain = vkbSwapchain.swapchain;
		// _swapchainImages = vkbSwapchain.get_images().value();
		// _swapchainImageViews = vkbSwapchain.get_image_views().value();
		//
		// _swapchainImageFormat = vkbSwapchain.image_format;
		
		rhi::SwapChainInfo swapChainInfo;
		swapChainInfo.sync = true;
		swapChainInfo.buffersCount = 2;
		swapChainInfo.height = _windowExtent.height;
		swapChainInfo.width = _windowExtent.width;
		rhi::SwapChain swapChain;
		// Memory leak. Only for test. In the future renderer will be completely rewritten using rhi
		_eRhi->create_swap_chain(&swapChain, &swapChainInfo);
		IVulkanSwapChain* vulkanSwapChain = static_cast<IVulkanSwapChain*>(swapChain.handle);
		_swapchain = vulkanSwapChain->get_swap_chain();
		_swapchainImages = vulkanSwapChain->get_images();
		_swapchainImageViews = vulkanSwapChain->get_image_views();
		_swapchainImageFormat = vulkanSwapChain->get_format();

		VkExtent3D imageExtent = {
			_windowExtent.width,
			_windowExtent.height,
			1
		};

		create_attachment(
			_transparencyColorAttach,
			imageExtent,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);

		create_attachment(
			_transparencyDepthAttach,
			imageExtent,
			VK_FORMAT_D32_SFLOAT,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT);

		create_attachment(
			_transparencyVelocityAttach,
			imageExtent,
			VK_FORMAT_R16G16_SFLOAT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);

		// G-buffer attachments
		create_attachment(
			_GBuffer.albedo,
			imageExtent,
			VK_FORMAT_A2R10G10B10_UNORM_PACK32,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);

		create_attachment(
			_GBuffer.normal,
			imageExtent,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);

		create_attachment(
			_GBuffer.surface,
			imageExtent,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);

		create_attachment(
			_GBuffer.depth,
			imageExtent,
			VK_FORMAT_D32_SFLOAT,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT);
	
		create_attachment(
			_GBuffer.velocity,
			imageExtent,
			VK_FORMAT_R16G16_SFLOAT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);

		create_attachment(
			_deferredColorAttach,
			imageExtent,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);

		create_attachment(
			_temporalFilter.taaCurrentColorAttach,
			imageExtent,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);

		create_attachment(
			_temporalFilter.taaOldColorAttach,
			imageExtent,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);

		create_attachment(
			_composite.colorAttach,
			imageExtent,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);

		create_attachment(
			_composite.velocityAttach,
			imageExtent,
			VK_FORMAT_R16G16_SFLOAT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);
	
		VkSamplerCreateInfo imgSamplerInfo = vkinit::sampler_create_info(VK_FILTER_LINEAR);
		// imgSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		// imgSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		// imgSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		// imgSamplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		// imgSamplerInfo.anisotropyEnable = VK_TRUE;
		// imgSamplerInfo.maxAnisotropy = 1.0f;
		// vkCreateSampler(_device, &imgSamplerInfo, nullptr, &_linearSampler);

		VkSamplerCreateInfo imgSamplerInfo2 = imgSamplerInfo;
		imgSamplerInfo2.magFilter = VK_FILTER_NEAREST;
		imgSamplerInfo2.minFilter = VK_FILTER_NEAREST;
		VkSamplerReductionModeCreateInfoEXT reductionInfo{};
		reductionInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT;
		reductionInfo.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;

		imgSamplerInfo2.pNext = &reductionInfo;
		vkCreateSampler(_device, &imgSamplerInfo2, nullptr, &_nearestSampler);

		/*_mainDeletionQueue.push_function([=](){
			vkDestroySampler(_device, _mainOpaqueSampler, nullptr);
		});*/

		VmaAllocationCreateInfo img_allocinfo{};
		img_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		img_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);	// VMA allocate 
	
		// create image for order independent transparency
		VkImageCreateInfo headIndexImageInfo = vkinit::image_create_info(VK_FORMAT_R32_UINT,
			(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT),
			imageExtent);

		Texture& headIndex = _transparencyData.headIndex;
		vmaCreateImage(_allocator, &headIndexImageInfo, &img_allocinfo, &headIndex.imageData.image, &headIndex.imageData.allocation, nullptr);

		VkImageViewCreateInfo viewInfo = vkinit::imageview_create_info(VK_FORMAT_R32_UINT, _transparencyData.headIndex.imageData.image, VK_IMAGE_ASPECT_COLOR_BIT);
		VK_CHECK(vkCreateImageView(_device, &viewInfo, nullptr, &_transparencyData.headIndex.imageView));	

		// setup depth pyramid to make culling
		_depthPyramidWidth = previous_pow2(_windowExtent.width);
		_depthPyramidHeight = previous_pow2(_windowExtent.height);
		_depthPyramidLevels = get_image_mip_levels(_depthPyramidWidth, _depthPyramidHeight);

		VkExtent3D pyramidExtent = {
			static_cast<uint32_t>(_depthPyramidWidth),
			static_cast<uint32_t>(_depthPyramidHeight),
			1
		};
	
		VkImageCreateInfo pyramidImageInfo = vkinit::image_create_info(VK_FORMAT_R32_SFLOAT,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			pyramidExtent);

		pyramidImageInfo.mipLevels = _depthPyramidLevels;

		vmaCreateImage(_allocator, &pyramidImageInfo, &img_allocinfo, &_depthPyramid.imageData.image, &_depthPyramid.imageData.allocation, nullptr);

		VkImageViewCreateInfo pyramidImageViewInfo = vkinit::imageview_create_info(VK_FORMAT_R32_SFLOAT,
			_depthPyramid.imageData.image,
			VK_IMAGE_ASPECT_COLOR_BIT);
		pyramidImageViewInfo.subresourceRange.levelCount = _depthPyramidLevels;
		VK_CHECK(vkCreateImageView(_device, &pyramidImageViewInfo, nullptr, &_depthPyramid.imageView));

		immediate_submit([=](VkCommandBuffer cmd){
			VkImageMemoryBarrier barrier1 = vkinit::image_barrier(
				_depthPyramid.imageData.image,
				0,
				VK_ACCESS_SHADER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_GENERAL,
				VK_IMAGE_ASPECT_COLOR_BIT);

			VkImageMemoryBarrier barrier2 = vkinit::image_barrier(
				_transparencyData.headIndex.imageData.image,
				0,
				VK_ACCESS_SHADER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_GENERAL,
				VK_IMAGE_ASPECT_COLOR_BIT);

			VkImageMemoryBarrier barrier3 = vkinit::image_barrier(
				_temporalFilter.taaOldColorAttach.imageData.image,
				0,
				VK_ACCESS_SHADER_READ_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT);

			std::vector<VkImageMemoryBarrier> barriers = { barrier1, barrier2, barrier3 };

			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, barriers.size(), barriers.data());
		});

		for (int32_t i = 0; i != _depthPyramidLevels; ++i)
		{
			VkImageViewCreateInfo viewInfo = vkinit::imageview_create_info(VK_FORMAT_R32_SFLOAT,
				_depthPyramid.imageData.image,
				VK_IMAGE_ASPECT_COLOR_BIT);
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseMipLevel = i;

			VkImageView pyramid;
			VK_CHECK(vkCreateImageView(_device, &viewInfo, nullptr, &pyramid));

			_depthPyramideMips[i] = pyramid;
			assert(_depthPyramideMips[i]);
		}

		VkSamplerCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.magFilter = VK_FILTER_LINEAR;
		createInfo.minFilter = VK_FILTER_LINEAR;
		createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.minLod = 0.0f;
		createInfo.maxLod = 16.0f;

		// This extension take 2x2 texel quad instead of taking more texels for linear filtering
		reductionInfo = {};
		reductionInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT;
		reductionInfo.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;

		createInfo.pNext = &reductionInfo;

		VK_CHECK(vkCreateSampler(_device, &createInfo, nullptr, &_depthSampler));
	}

	void VkRenderer::init_commands()
	{
		VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily, 
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		for (int i = 0; i != FRAME_OVERLAP; ++i)
		{
			VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));
			VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);

			VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));

			_mainDeletionQueue.push_function([=](){
				vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);
			});
		}

		VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily);
		VK_CHECK(vkCreateCommandPool(_device, &uploadCommandPoolInfo, nullptr, &_uploadContext._commandPool));
		_mainDeletionQueue.push_function([=](){
			vkDestroyCommandPool(_device, _uploadContext._commandPool, nullptr);
		});

		VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_uploadContext._commandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_uploadContext._commandBuffer));
	}

	void VkRenderer::init_renderpasses()
	{
		RenderPassBuilder::begin()
			.add_color_attachment(_transparencyColorAttach.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.add_color_attachment(_transparencyVelocityAttach.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.add_depth_attachment(_transparencyDepthAttach.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.add_subpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
			.build(_device, _transparencyRenderPass);
	
		RenderPassBuilder::begin()
			.add_color_attachment(_swapchainImageFormat, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
			.add_subpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
			.build(_device, _renderPass);

		RenderPassBuilder::begin()
			.add_color_attachment(_GBuffer.albedo.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.add_color_attachment(_GBuffer.normal.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.add_color_attachment(_GBuffer.surface.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.add_color_attachment(_GBuffer.velocity.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.add_depth_attachment(_GBuffer.depth.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.add_subpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
			.build(_device, _GBuffer.renderPass);

		RenderPassBuilder::begin()
			.add_color_attachment(_deferredColorAttach.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.add_subpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
			.build(_device, _deferredRenderPass);

		RenderPassBuilder::begin()
			.add_color_attachment(_temporalFilter.taaCurrentColorAttach.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.add_subpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
			.build(_device, _temporalFilter.taaRenderPass);

		RenderPassBuilder::begin()
			.add_color_attachment(_composite.colorAttach.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.add_color_attachment(_composite.velocityAttach.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.add_subpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
			.build(_device, _composite.renderPass);

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		VkSubpassDependency dependency{};
		//dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;	// Maybe it's not necessary to dependent on the previous subpass
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = 0;
	
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.attachmentCount = 0;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
		VK_CHECK(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_transparencyData.renderPass));
		
		_mainDeletionQueue.push_function([=](){
			vkDestroyRenderPass(_device, _composite.renderPass, nullptr);
			vkDestroyRenderPass(_device, _temporalFilter.taaRenderPass, nullptr);
			vkDestroyRenderPass(_device, _deferredRenderPass, nullptr);
			vkDestroyRenderPass(_device, _transparencyData.renderPass, nullptr);
			vkDestroyRenderPass(_device, _renderPass, nullptr);
			vkDestroyRenderPass(_device, _transparencyRenderPass, nullptr);
			vkDestroyRenderPass(_device, _GBuffer.renderPass, nullptr);
		});
	}

	/** Used to init framebuffers which are based on ths size of the main viewport.
	*/
	void VkRenderer::init_framebuffers()
	{
		LOG_INFO("Init framebuffers");
		VkFramebufferCreateInfo fbInfo{};
		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.pNext = nullptr;

		fbInfo.attachmentCount = 1;
		fbInfo.width = _windowExtent.width;
		fbInfo.height = _windowExtent.height;
		fbInfo.layers = 1;

		const uint32_t swapchainImagecount = _swapchainImages.size();
		_framebuffers = std::vector<VkFramebuffer>(swapchainImagecount);

		_mainOpaqueFramebuffers = std::vector<VkFramebuffer>(swapchainImagecount);

		for (int i = 0; i != swapchainImagecount; ++i)
		{
			VkImageView attachments[2];
			attachments[0] = _swapchainImageViews[i];

			fbInfo.renderPass = _renderPass;
			fbInfo.pAttachments = &attachments[0];
			fbInfo.attachmentCount = 1;
			VK_CHECK(vkCreateFramebuffer(_device, &fbInfo, nullptr, &_framebuffers[i]));
		}

		fbInfo.width = _windowExtent.width;
		fbInfo.height = _windowExtent.height;
		fbInfo.attachmentCount = 0;
		fbInfo.layers = 1;
		fbInfo.renderPass = _transparencyData.renderPass;
		VK_CHECK(vkCreateFramebuffer(_device, &fbInfo, nullptr, &_transparencyData.framebuffer));

		//VkImageView imageViews[2] = { _transparencyColorAttach.imageView, _transparencyDepthAttach.imageView };
		std::vector<VkImageView> imageViews = {
			_transparencyColorAttach.imageView,
			_transparencyVelocityAttach.imageView,
			_transparencyDepthAttach.imageView };
	
		fbInfo.attachmentCount = imageViews.size();
		fbInfo.pAttachments = imageViews.data();
		fbInfo.renderPass = _transparencyRenderPass;
		VK_CHECK(vkCreateFramebuffer(_device, &fbInfo, nullptr, &_transparencyFramebuffer));
		imageViews.clear();

		imageViews = { _GBuffer.albedo.imageView, _GBuffer.normal.imageView, _GBuffer.surface.imageView, _GBuffer.velocity.imageView, _GBuffer.depth.imageView };
		fbInfo.attachmentCount = imageViews.size();
		fbInfo.pAttachments = imageViews.data();
		fbInfo.renderPass = _GBuffer.renderPass;
		VK_CHECK(vkCreateFramebuffer(_device, &fbInfo, nullptr, &_GBuffer.framebuffer));
		imageViews.clear();

		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = &_deferredColorAttach.imageView;
		fbInfo.renderPass = _deferredRenderPass;
		VK_CHECK(vkCreateFramebuffer(_device, &fbInfo, nullptr, &_deferredFramebuffer));
	
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = &_temporalFilter.taaCurrentColorAttach.imageView;
		fbInfo.renderPass = _temporalFilter.taaRenderPass;
		VK_CHECK(vkCreateFramebuffer(_device, &fbInfo, nullptr, &_temporalFilter.taaFramebuffer));

		imageViews = { _composite.colorAttach.imageView, _composite.velocityAttach.imageView };
		fbInfo.attachmentCount = imageViews.size();
		fbInfo.pAttachments = imageViews.data();
		fbInfo.renderPass = _composite.renderPass;
		VK_CHECK(vkCreateFramebuffer(_device, &fbInfo, nullptr, &_composite.framebuffer));
	}

	void VkRenderer::init_shadow_maps()
	{	
		VkExtent3D lightShadowMapExtent = {
			2048,
			2048,
			1
		};

		actors::DirectionalLight dirLight;
		float nearPlane = 0.01f, farPlane = 600.0f;
		dirLight.direction = glm::vec4(-25.0f, -75.0f, -25.0f, 0.0f);
		dirLight.direction = glm::normalize(dirLight.direction);

		dirLight.colorAndIntensity = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
		_renderScene._dirLights.push_back(dirLight);

		actors::PointLight pointLight;
		pointLight.sourceRadius = 0.0f;
		pointLight.colorAndIntensity = glm::vec4(1.0f, 0.0f, 0.0f, 5000.0f);
		pointLight.positionAndAttRadius = glm::vec4(30.0f, 30.0f, 25.0f, 1000.0f);
		_renderScene._pointLights.push_back(pointLight);

		actors::SpotLight spotLight;
		spotLight.colorAndIntensity = glm::vec4(0.0f, 0.0f, 1.0f, 5000.0f);
		spotLight.positionAndDistance = glm::vec4(13.0f, 8.0f, 9.0f, 300.0f);
		float innerConeRadius = glm::cos(glm::radians(10.0f));
		float rx = glm::radians(10.0f);
		float ry = glm::radians(25.0f);
		float rz = glm::radians(0.0f);
		spotLight.rotationAndInnerConeRadius = glm::vec4(rx, ry, rz, innerConeRadius);
		spotLight.outerConeRadius = glm::cos(glm::radians(65.0f));
		_renderScene._spotLights.push_back(spotLight);

		for (int i = 0; i != _renderScene._dirLights.size(); ++i)
		{
			ShadowMap tempMap;
			create_attachment(
				tempMap.attachment,
				lightShadowMapExtent,
				VK_FORMAT_D32_SFLOAT,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_IMAGE_ASPECT_DEPTH_BIT
			);

			actors::DirectionalLight& dirLight = _renderScene._dirLights[i];

			ShadowMap::create_light_space_matrices(this, ActorType::DirectionalLight, i, tempMap);

			RenderPassBuilder::begin()
				.add_depth_attachment(tempMap.attachment.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL)
				.add_subpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
				.build(_device, tempMap.renderPass);

			VkImageView* attach = &tempMap.attachment.imageView;
			VkFramebufferCreateInfo info = vkinit::framebuffer_create_info(attach, 1, tempMap.renderPass, tempMap.attachment.extent);
			VK_CHECK(vkCreateFramebuffer(_device, &info, nullptr, &tempMap.framebuffer));

			_renderScene._dirShadowMaps.push_back(tempMap);

			_mainDeletionQueue.push_function([=](){
				ShadowMap::destroy_shadow_map(this, tempMap);
			});
		}

		VkSamplerCreateInfo samplerInfo = vkinit::sampler_create_info(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VK_CHECK(vkCreateSampler(_device, &samplerInfo, nullptr, &_shadowMapSampler));

		_mainDeletionQueue.push_function([=](){
			vkDestroySampler(_device, _shadowMapSampler, nullptr);
		});

		for (int i = 0; i != _renderScene._pointLights.size(); ++i)
		{
			ShadowMap tempMap;
			actors::PointLight& pointLight = _renderScene._pointLights[i];

			ShadowMap::create_light_space_matrices(this, ActorType::PointLight, i, tempMap);
		
			Attachment& attachment = tempMap.attachment;
			attachment.format = VK_FORMAT_D32_SFLOAT;
			attachment.extent = lightShadowMapExtent;
			create_cube_map(
				attachment,
				attachment.extent,
				attachment.format,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_IMAGE_ASPECT_DEPTH_BIT);

			const uint32_t viewMask = 0b111111;
			const uint32_t correlationMask = 0b00111111;

			VkRenderPassMultiviewCreateInfoKHR renderPassMultiviewInfo{};
			renderPassMultiviewInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO_KHR;
			renderPassMultiviewInfo.subpassCount = 1;
			renderPassMultiviewInfo.pViewMasks = &viewMask;
			renderPassMultiviewInfo.correlationMaskCount = 1;
			renderPassMultiviewInfo.pCorrelationMasks = &correlationMask;

			VkRenderPass& renderPass = tempMap.renderPass;
			RenderPassBuilder::begin()
				.add_depth_attachment(attachment.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL)
				.add_subpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
				.build(_device, renderPass, &renderPassMultiviewInfo);

			VkFramebufferCreateInfo framebufferInfo = vkinit::framebuffer_create_info(
				&attachment.imageView,
				1,
				renderPass,
				attachment.extent);

			VkFramebuffer& framebuffer = tempMap.framebuffer;
			VK_CHECK(vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &framebuffer));

			_renderScene._pointShadowMaps.push_back(tempMap);
		
			_mainDeletionQueue.push_function([=](){
				ShadowMap::destroy_shadow_map(this, tempMap);
			});
		}

		for (int i = 0; i != _renderScene._spotLights.size(); ++i)
		{
			ShadowMap tempMap;
			actors::SpotLight& spotLight = _renderScene._spotLights[i];

			ShadowMap::create_light_space_matrices(this, ActorType::SpotLight, i, tempMap);
		
			create_attachment(
				tempMap.attachment,
				lightShadowMapExtent,
				VK_FORMAT_D32_SFLOAT,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_IMAGE_ASPECT_DEPTH_BIT
			);
		
			RenderPassBuilder::begin()
				.add_depth_attachment(tempMap.attachment.format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL)
				.add_subpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
				.build(_device, tempMap.renderPass);

			VkImageView* attach = &tempMap.attachment.imageView;
			VkFramebufferCreateInfo info = vkinit::framebuffer_create_info(attach, 1, tempMap.renderPass, tempMap.attachment.extent);
			VK_CHECK(vkCreateFramebuffer(_device, &info, nullptr, &tempMap.framebuffer));

			_renderScene._spotShadowMaps.push_back(tempMap);

			_mainDeletionQueue.push_function([=](){
				ShadowMap::destroy_shadow_map(this, tempMap);
			});
		}
	}

	void VkRenderer::init_sync_structures()
	{  
		LOG_INFO("Init sync struct");
		VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
	
		VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

		for (int i = 0; i != FRAME_OVERLAP; ++i)
		{
			VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));

			_mainDeletionQueue.push_function([=](){
				vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
			});

			VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._presentSemaphore));
			VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));
		}

		VkFenceCreateInfo uploadFenceCreateInfo = vkinit::fence_create_info();
		VK_CHECK(vkCreateFence(_device, &uploadFenceCreateInfo, nullptr, &_uploadContext._uploadFence));
		_mainDeletionQueue.push_function([=](){
			vkDestroyFence(_device, _uploadContext._uploadFence, nullptr);
		});
	}

	void VkRenderer::init_buffers()
	{
		LOG_INFO("Init global buffers");
		//const size_t sceneParamBufferSize = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUSceneData));
		//_sceneParameterBuffer = create_buffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		LOG_INFO("Before first loop")
		for (int i = 0; i != FRAME_OVERLAP; ++i)
		{
			_frames[i]._cameraBuffer = create_buffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

			_frames[i]._sceneDataBuffer.create_buffer(this, sizeof(GPUSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

			_frames[i]._settingsBuffer.create_buffer(
				this,
				sizeof(Settings),
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VMA_MEMORY_USAGE_CPU_TO_GPU);
		
			VkDescriptorImageInfo offscrColorImgInfo;
			offscrColorImgInfo.sampler = _linearSampler;
			offscrColorImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			offscrColorImgInfo.imageView = _mainOpaqueColorAttach.imageView;
		}
		LOG_INFO("Before second loop")
		for (int i = 0; i != FRAME_OVERLAP; ++i)
		{
			_mainDeletionQueue.push_function([=](){
				vmaDestroyBuffer(_allocator, _frames[i]._sceneDataBuffer._buffer, _frames[i]._sceneDataBuffer._allocation);
				vmaDestroyBuffer(_allocator, _frames[i]._cameraBuffer._buffer, _frames[i]._cameraBuffer._allocation);
				vmaDestroyBuffer(_allocator, _frames[i]._settingsBuffer._buffer, _frames[i]._settingsBuffer._allocation);
			});
		}

		GeometryInfo geometryInfo;
		geometryInfo.count = 0;
		geometryInfo.maxNodeCount = NODE_COUNT * _windowExtent.width * _windowExtent.height;

		AllocatedBufferT<GeometryInfo> stagingBuffer(this, sizeof(GeometryInfo), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		stagingBuffer.copy_from(this, &geometryInfo, sizeof(GeometryInfo));
		LOG_INFO("Before transparency geometry info")
		_transparencyData.geometryInfo.create_buffer(
			this,
			sizeof(GeometryInfo),
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY);
	
		immediate_submit([&](VkCommandBuffer cmd){
			AllocatedBufferT<GeometryInfo>::copy_typed_buffer_cmd(this, cmd, &stagingBuffer, &_transparencyData.geometryInfo);
		});

		stagingBuffer.destroy_buffer(this);
		LOG_INFO("Before transparency nodes buffer")
		_transparencyData.nodes.create_buffer(
			this,
			sizeof(Node) * geometryInfo.maxNodeCount,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY);
	
		// AllocatedBuffer tempBuffer(
		// 	this,
		// 	sizeof(TemporalFilter::Jittering),
		// 	VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		// 	VMA_MEMORY_USAGE_CPU_ONLY);
		//
		// tempBuffer.copy_from(this, &_temporalFilter.jittering, sizeof(TemporalFilter::Jittering));
		//
		LOG_INFO("Before jittering buffer")
		_temporalFilter.jitteringBuffer.create_buffer(
			this,
			sizeof(TemporalFilter::Jittering),
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU);

		_temporalFilter.jitteringBuffer.copy_from(this, &_temporalFilter.jittering, sizeof(TemporalFilter::Jittering));

		// immediate_submit([&](VkCommandBuffer cmd){
		// 	AllocatedBuffer::copy_buffer_cmd(this, cmd, &tempBuffer, &_temporalFilter.jitteringBuffer);
		// });

		//tempBuffer.destroy_buffer(this);
	
		_mainDeletionQueue.push_function([=](){
			vmaDestroyBuffer(_allocator, _transparencyData.geometryInfo._buffer, _transparencyData.geometryInfo._allocation);
			vmaDestroyBuffer(_allocator, _transparencyData.nodes._buffer, _transparencyData.nodes._allocation);
			vmaDestroyBuffer(_allocator, _temporalFilter.jitteringBuffer._buffer, _temporalFilter.jitteringBuffer._allocation);
		});
		LOG_INFO("Before setting up pipeline build of transparency data")
		_transparencyData.setup_pipeline_builder(this);
		LOG_INFO("Before setting up transparency data shader pass")
		_transparencyData.create_shader_pass(this);
	}

	void VkRenderer::init_pipelines()
	{
		LOG_INFO("Init pipelines")
		Shader depthReduceShader(_device);
		rhi::ShaderInfo depthReduceInfo;
		io::URI depthReduceURI("shaders/compute/reduce_depth.comp");
		_shaderCompiler->compile_into_spv(depthReduceURI, &depthReduceInfo);

		rhi::Shader shader;
		_eRhi->create_shader(&shader, &depthReduceInfo);
		rhi::ComputePipelineInfo pipeInfo;
		pipeInfo.shaderStage = shader;
		rhi::Pipeline pipeline;
		_eRhi->create_compute_pipeline(&pipeline, &pipeInfo);
		LOG_INFO("Compiled shader {} into spv", depthReduceURI.c_str())

		IVulkanPipeline* vkPipeline = static_cast<IVulkanPipeline*>(pipeline.handle);
		_depthReducePipeline = vkPipeline->get_handle();
		_depthReduceLayout = vkPipeline->get_layout();
		
		//depthReduceShader.load_shader_module(depthReduceInfo);
		
		Shader drawCullShader(_device);
		rhi::ShaderInfo drawCullInfo;
		io::URI drawCullURI("shaders/compute/draw_cull.comp");
		_shaderCompiler->compile_into_spv(drawCullURI, &drawCullInfo);
		drawCullShader.load_shader_module(drawCullInfo);
		LOG_INFO("Compiled shader {} into spv", drawCullURI.c_str())
		//setup_compute_pipeline(&depthReduceShader, _depthReducePipeline, _depthReduceLayout);
		setup_compute_pipeline(&drawCullShader, _cullingPipeline, _cullintPipelineLayout);

		_mainDeletionQueue.push_function([&](){
			vkDestroyPipeline(_device, _depthReducePipeline, nullptr);
			vkDestroyPipelineLayout(_device, _depthReduceLayout, nullptr);

			vkDestroyPipeline(_device, _cullingPipeline, nullptr);
			vkDestroyPipelineLayout(_device, _cullintPipelineLayout, nullptr);
		});

		//depthReduceShader.delete_shader_module();
		drawCullShader.delete_shader_module();
	}

	void VkRenderer::setup_compute_pipeline(Shader* shader, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout)
	{
		ShaderEffect tempEffect;
		tempEffect.add_stage(shader, VK_SHADER_STAGE_COMPUTE_BIT);
		pipelineLayout = tempEffect.get_pipeline_layout(_device);

		ShaderEffect::ShaderStage stage;
		stage.shader = shader;
		stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;

		ComputePipelineBuilder builder(_device);
	
		pipeline = builder.build(stage, pipelineLayout);
	}

	void VkRenderer::init_output_quad()
	{
		size_t outputQuadVertBufferSize = _outputQuad._vertices.size() * sizeof(Vertex);
		AllocatedBuffer vertexStagingBuffer = create_buffer(outputQuadVertBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	
		void* data;
		vmaMapMemory(_allocator, vertexStagingBuffer._allocation, &data);
		memcpy(data, _outputQuad._vertices.data(), outputQuadVertBufferSize);
		vmaUnmapMemory(_allocator, vertexStagingBuffer._allocation);

		_outputQuad._vertexBuffer = create_buffer(outputQuadVertBufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		immediate_submit([=](VkCommandBuffer cmd){
			VkBufferCopy copy;
			copy.dstOffset = 0;
			copy.srcOffset = 0;
			copy.size = outputQuadVertBufferSize;
			vkCmdCopyBuffer(cmd, vertexStagingBuffer._buffer, _outputQuad._vertexBuffer._buffer, 1, &copy);
		});

		_mainDeletionQueue.push_function([=](){
			vmaDestroyBuffer(_allocator, _outputQuad._vertexBuffer._buffer, _outputQuad._vertexBuffer._allocation);
		});

		vmaDestroyBuffer(_allocator, vertexStagingBuffer._buffer, vertexStagingBuffer._allocation);

		MaterialData materialData;
		materialData.baseTemplate = "Postprocessing";
		_materialSystem.build_material("Postprocessing", materialData);
	}

	void VkRenderer::run()
	{
		bool bQuit = false;

		//main loop
		while (!bQuit)
		{
			_sdlWindow.handle_action_per_frame(&camera, &bQuit);

			//ImGui::ShowDemoWindow();
			_userInterface.draw_ui(this);
			draw();
		}
	}

	void VkRenderer::draw()
	{
		_userInterface.render_ui();
		ImGui::Render();
		LOG_INFO("Start new frame")

		LOG_INFO("Width {} Height {}", _windowExtent.width, _windowExtent.height)

		VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

		// Wait for GPU
		VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000));
		VK_CHECK(vkResetFences(_device, 1, &get_current_frame()._renderFence));
		get_current_frame()._frameDeletionQueue.flush();
		get_current_frame()._dynamicDescriptorAllocator.reset_pools();

		uint32_t swapchainImageIndex;

		if (vkAcquireNextImageKHR(_device, _swapchain, 1000000000, get_current_frame()._presentSemaphore, nullptr, &swapchainImageIndex) == VK_ERROR_OUT_OF_DATE_KHR)
		{
			refresh_swapchain();
			return;
		}

		VkCommandBufferBeginInfo cmdBeginInfo{};
		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBeginInfo.pNext = nullptr;

		cmdBeginInfo.pInheritanceInfo = nullptr;
		cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK(vkBeginCommandBuffer(get_current_frame()._mainCommandBuffer, &cmdBeginInfo));

		prepare_per_frame_data(cmd);

		prepare_data_for_drawing(cmd);		// Prepare per mesh pass data

		// Copy indirect buffer data from host buffer to device buffer
		prepare_gpu_indirect_buffer(cmd, _renderScene._pointShadowPass);
		prepare_gpu_indirect_buffer(cmd, _renderScene._dirShadowPass);
		prepare_gpu_indirect_buffer(cmd, _renderScene._deferredPass);
		prepare_gpu_indirect_buffer(cmd, _renderScene._spotShadowPass);
		prepare_gpu_indirect_buffer(cmd, _renderScene._transparentForwardPass);
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, _beforeCullingBufferBarriers.size(), _beforeCullingBufferBarriers.data(), 0, nullptr);
		_beforeCullingBufferBarriers.clear();

		bake_shadow_maps(cmd);
		if (!_afterShadowsBarriers.empty())
		{
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, _afterShadowsBarriers.size(), _afterShadowsBarriers.data());
			_afterShadowsBarriers.clear();
		}
		
		draw_deferred_pass(cmd);
		draw_tranparency_pass(cmd);
		draw_compositing_pass(cmd);
		draw_taa_pass(cmd);
		draw_final_quad(cmd, swapchainImageIndex);
		
		depth_reduce(cmd);

		VK_CHECK(vkEndCommandBuffer(get_current_frame()._mainCommandBuffer));
		submit(cmd, swapchainImageIndex);
	
		LOG_INFO("Frame number is {}", _frameNumber)
		_frameNumber++;
	}

	void VkRenderer::parse_prefabs()
	{
		LOG_INFO("Parsing prefabs")
		_materialSystem.build_default_templates();
		std::string assetsPath = _projectPath + "/assets/";
		fs::path direcory{ assetsPath };
	
		std::vector<Mesh> meshes;
	
		VkSamplerCreateInfo samplerInfo = vkinit::sampler_create_info(VK_FILTER_LINEAR);
		VK_CHECK(vkCreateSampler(_device, &samplerInfo, nullptr, &_textureSampler));

		std::vector<glm::vec3> translation = {
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(-100.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f)
		};

		int counter = 0;
		for (auto& p : fs::directory_iterator(direcory))
		{
			if (p.path().extension() == ".pref")
			{
				assets::AssetFile file;
				assets::load_binaryFile(p.path().string().c_str(), file);
				assets::PrefabInfo info = assets::read_prefab_info(&file);

				Mesh tempMesh{};
				tempMesh.load_from_mesh_asset(info.meshPath.c_str());
				glm::mat4 model = glm::translate(glm::mat4(1.0f), translation[counter]);

				meshes.push_back(tempMesh);
				std::string meshName = info.meshPath;
				meshName.erase(0, meshName.find_last_of("/"));
				meshName.erase(meshName.find(".mesh"), 5);
				meshName.erase(meshName.find('/'), 1);
				_meshes[meshName] = tempMesh;

				assets::load_binaryFile(info.materialPath.c_str(), file);
				assets::MaterialInfo materialInfo = assets::read_material_info(&file);
				MaterialData materialData;
				materialData.baseTemplate = materialInfo.baseEffect;
				MeshObject tempMeshObject;

				if (!materialInfo.textures.empty())
				{
					for (auto& tex : materialInfo.textures)
					{
						Texture tempTexture;
						load_image_from_asset(*this, tex.second.c_str(), tempTexture.imageData);
						VkImageViewCreateInfo imageViewInfo = vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_UNORM, tempTexture.imageData.image, VK_IMAGE_ASPECT_COLOR_BIT);
						vkCreateImageView(_device, &imageViewInfo, nullptr, &tempTexture.imageView);

						_mainDeletionQueue.push_function([=](){
							vkDestroyImageView(_device, tempTexture.imageView, nullptr);
						});

						if (tex.first == "texture_base_color")
						{
							_baseColorTextures.push_back(tempTexture);
						
							tempMeshObject.baseColor = _baseColorTextures.back().imageView;
						}
						else if (tex.first == "texture_normal")
						{
							_normalTextures.push_back(tempTexture);
						
							tempMeshObject.normal = _normalTextures.back().imageView;
						}
						else if (tex.first == "texture_arm")
						{
							_armTextures.push_back(tempTexture);
						
							tempMeshObject.arm = _armTextures.back().imageView;
						}
					}
				}
				else
				{
					LOG_WARNING("No textures in {} material", materialInfo.materialName)
				}

				if (materialInfo.mode == assets::MaterialMode::TRANSPARENT)
				{
					tempMeshObject.bDrawForwardPass = 0;
					tempMeshObject.bDrawSpotShadowPass = 0;
					tempMeshObject.bDrawPointShadowPass = 0;
					tempMeshObject.bDrawShadowPass = 0;
					tempMeshObject.bDrawTransparencyPass = 1;
				}
			
				_materialSystem.build_material(materialInfo.materialName, materialData);
				tempMeshObject.material = _materialSystem.get_material(materialInfo.materialName);
				tempMeshObject.mesh = &_meshes[meshName];
				tempMeshObject.transformMatrix = model;
				tempMeshObject.bDrawDeferredPass = 1;
				++counter;
			
				_meshObjects.push_back(tempMeshObject);
			}
		}

		_mainDeletionQueue.push_function([=](){
			vkDestroySampler(_device, _textureSampler, nullptr);
		});

		fill_renderable_objects();
	}

	FrameData& VkRenderer::get_current_frame()
	{
		return _frames[_frameNumber % FRAME_OVERLAP];
	}

	size_t VkRenderer::pad_uniform_buffer_size(size_t originalSize)
	{
		size_t minUboAlignment = _gpuProperties.limits.minUniformBufferOffsetAlignment;
		size_t alignedSize = originalSize;
		if (minUboAlignment > 0)
		{
			alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}
		return alignedSize;
	}

	void VkRenderer::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function)
	{
		VkCommandBuffer cmd = _uploadContext._commandBuffer;

		VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

		function(cmd);

		VK_CHECK(vkEndCommandBuffer(cmd));

		VkSubmitInfo submit = vkinit::submit_info(&cmd);

		VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, _uploadContext._uploadFence));

		vkWaitForFences(_device, 1, &_uploadContext._uploadFence, true, 9999999999);
		vkResetFences(_device, 1, &_uploadContext._uploadFence);

		vkResetCommandPool(_device, _uploadContext._commandPool, 0);
	}

	void VkRenderer::create_attachment(
		Attachment& attachment,
		VkExtent3D imageExtent,
		VkFormat format, 
		VkImageUsageFlags usageFlags,
		VkImageAspectFlags aspectFlags,
		uint32_t layerCount)
	{
		attachment.format = format;
		attachment.extent = imageExtent;

		VkImageCreateInfo img_info = vkinit::image_create_info(attachment.format, usageFlags, imageExtent);
		img_info.arrayLayers = layerCount;
	
		VmaAllocationCreateInfo img_allocinfo{};
		img_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		img_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);	// VMA allocate the image into VRAM in all situations

		vmaCreateImage(_allocator, &img_info, &img_allocinfo, &attachment.imageData.image, &attachment.imageData.allocation, nullptr);

		VkImageViewCreateInfo viewInfo = vkinit::imageview_create_info(attachment.format, attachment.imageData.image, aspectFlags);
		if (layerCount != 1)
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.subresourceRange.layerCount = layerCount;
		VK_CHECK(vkCreateImageView(_device, &viewInfo, nullptr, &attachment.imageView));
	}

	void VkRenderer::create_cube_map(
		Texture& texture,
		VkExtent3D imageExtent,
		VkFormat format,
		VkImageUsageFlags usageFlags,
		VkImageAspectFlags aspectFlags)
	{
		VkImage& image = texture.imageData.image;
		VmaAllocation& allocation = texture.imageData.allocation;

		VkImageCreateInfo imageInfo = vkinit::image_create_info(format, usageFlags, imageExtent);
		imageInfo.arrayLayers = 6;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VmaAllocationCreateInfo imageAllocInfo{};
		imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		imageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vmaCreateImage(_allocator, &imageInfo, &imageAllocInfo, &image, &allocation, nullptr);

		VkImageViewCreateInfo viewInfo = vkinit::imageview_create_info(format, image, aspectFlags);
		viewInfo.subresourceRange.layerCount = 6;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

		VkImageView tempView;
		VK_CHECK(vkCreateImageView(_device, &viewInfo, nullptr, &tempView));
		texture.imageView = tempView;
	}

	AllocatedBuffer VkRenderer::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = nullptr;

		bufferInfo.size = allocSize;
		bufferInfo.usage = usage;

		VmaAllocationCreateInfo vmaallocInfo{};
		vmaallocInfo.usage = memoryUsage;

		AllocatedBuffer newBuffer;

		VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo, &newBuffer._buffer, &newBuffer._allocation, nullptr));

		return newBuffer;
	}

	void VkRenderer::reallocate_buffer(AllocatedBuffer& buffer, size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
		AllocatedBuffer newBuffer(this, size, usage, memoryUsage);

		if (buffer._buffer != VK_NULL_HANDLE)
		{
			get_current_frame()._frameDeletionQueue.push_function([=](){
				vmaDestroyBuffer(_allocator, buffer._buffer, buffer._allocation);
			});
		}

		buffer = newBuffer;
	}

	void VkRenderer::refresh_multi_threads_command_buffers(uint32_t objectsPerThread)
	{
		for (auto& thread : _threadInfo)
		{
			if (thread.commandPool == VK_NULL_HANDLE)
				break;
			vkDestroyCommandPool(_device, thread.commandPool, nullptr);
		}

		VkCommandPoolCreateInfo commandPoolCreateInfo = vkinit::command_pool_create_info(_graphicsQueueFamily,
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	
		for (int i = 0; i != _numThreads; ++i)
		{
			ThreadInfo* thread = &_threadInfo[i];
			VK_CHECK(vkCreateCommandPool(_device, &commandPoolCreateInfo, nullptr, &thread->commandPool));
			VkCommandBufferAllocateInfo allocInfo = vkinit::command_buffer_allocate_info(thread->commandPool, objectsPerThread, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
			thread->commandBuffers.reserve(objectsPerThread);
			VK_CHECK(vkAllocateCommandBuffers(_device, &allocInfo, thread->commandBuffers.data()));
		}
	}

	void VkRenderer::refresh_swapchain()
	{
		LOG_INFO("Refreshing swap chain")
		LOG_INFO("Window before size is {}x{}", _windowExtent.width, _windowExtent.height)

		vkDeviceWaitIdle(_device);

		int width = 0, height = 0;
		SDL_GetWindowSize(_sdlWindow.get_window(), &width, &height);
		_windowExtent.width = (uint32_t)width;
		_windowExtent.height = (uint32_t)height;

		LOG_INFO("Window after size is {}x{}", _windowExtent.width, _windowExtent.height)

		LOG_INFO("Before deleting");
		for (int i = 0; i != _swapchainImages.size(); ++i)
		{
			vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
			vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
		}

		vkDestroyFramebuffer(_device, _transparencyFramebuffer, nullptr);
		vkDestroyFramebuffer(_device, _transparencyData.framebuffer, nullptr);
		vkDestroyFramebuffer(_device, _deferredFramebuffer, nullptr);
		//vkDestroyFramebuffer(_device, _compositeFramebuffer, nullptr);
	
		_transparencyColorAttach.destroy_attachment(this);
		_transparencyDepthAttach.destroy_attachment(this);
		_transparencyData.headIndex.destroy_texture(this);
		_deferredColorAttach.destroy_attachment(this);
		_transparencyVelocityAttach.destroy_attachment(this);
		_depthPyramid.destroy_texture(this);

		_GBuffer.cleanup(this);
		_temporalFilter.cleanup(this);
		_composite.cleanup(this);

		for (int i = 0; i != _depthPyramidLevels; ++i)
			vkDestroyImageView(_device, _depthPyramideMips[i], nullptr);

		vkDestroySampler(_device, _depthSampler, nullptr);
		//vkDestroySampler(_device, _linearSampler, nullptr);
		vkDestroySampler(_device, _nearestSampler, nullptr);
		vkDestroySwapchainKHR(_device, _swapchain, nullptr);
	
		init_swapchain();
		init_framebuffers();
		//_materialSystem.refresh_default_templates();
	}

	// functions for depth pyramid to make culling
	uint32_t previous_pow2(uint32_t v)
	{
		uint32_t r = 1;

		while (r * 2 < v)
			r *= 2;

		return r;
	}

	uint32_t get_image_mip_levels(uint32_t width, uint32_t height)
	{
		uint32_t result = 1;

		while (width > 1 || height > 1)
		{
			result++;
			width /= 2;
			height /= 2;
		}

		return result;
	}

	VertexInputDescription Plane::get_vertex_description()
	{
		VertexInputDescription description;
	
		VkVertexInputBindingDescription mainBinding{};
		mainBinding.binding = 0;
		mainBinding.stride = sizeof(Vertex);
		mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		description.bindings.push_back(mainBinding);

		VkVertexInputAttributeDescription positionAttribute{};
		positionAttribute.binding = 0;
		positionAttribute.location = 0;
		positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		positionAttribute.offset = offsetof(Vertex, position);

		VkVertexInputAttributeDescription uvAttribute{};
		uvAttribute.binding = 0;
		uvAttribute.location = 1;
		uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
		uvAttribute.offset = offsetof(Vertex, texCoord);

		description.attributes.push_back(positionAttribute);
		description.attributes.push_back(uvAttribute);

		return description;
	}

	void TransparencyFirstPassData::setup_pipeline_builder(VkRenderer* engine)
	{
		pipelineBuilder = GraphicsPipelineBuilder(engine->_device);
		auto description = Mesh::get_vertex_description();
		pipelineBuilder.setup_vertex_input_state(description);
		pipelineBuilder.setup_assembly_state(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipelineBuilder.setup_depth_state(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
		pipelineBuilder.setup_rasterization_state(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT);
		pipelineBuilder.setup_multisample_state();
		pipelineBuilder.setup_dynamic_viewport_state();
		pipelineBuilder.setup_color_blend_state(0xf);
		pipelineBuilder.setup_dynamic_state(true, false);
	}

	void TransparencyFirstPassData::create_shader_pass(VkRenderer* engine)
	{
		ShaderEffect* effect = engine->_materialSystem.build_shader_effect({
			"shaders/oit/oit_geometry.vert",
			"shaders/oit/oit_geometry.frag"
		});
		LOG_INFO("Before building shader pass")
		geometryPass = engine->_materialSystem.build_shader_pass(renderPass, pipelineBuilder, effect);
	}

	void TransparencyFirstPassData::cleanup(VkRenderer* engine)
	{
		VkDevice device = engine->_device;
		vkDestroyPipeline(device, geometryPass->pipeline, nullptr);
		vkDestroyPipelineLayout(device, geometryPass->layout, nullptr);
		geometryPass->effect->destroy_shader_modules();
	}

	void GBuffer::cleanup(VkRenderer* engine)
	{
		albedo.destroy_attachment(engine);
		normal.destroy_attachment(engine);
		surface.destroy_attachment(engine);
		depth.destroy_attachment(engine);
		velocity.destroy_attachment(engine);
		vkDestroyFramebuffer(engine->_device, framebuffer, nullptr);
	}

	void TemporalFilter::init(VkRenderer* engine)
	{
		prepare_jittering_data();
		jittering.haltonScale = 0.6f;
		jittering.numSamples = 36;

		MaterialData materialData;
		materialData.baseTemplate = "TAA";
		engine->_materialSystem.build_material("TAA", materialData);
	}

	void TemporalFilter::cleanup(VkRenderer* engine)
	{
		taaCurrentColorAttach.destroy_attachment(engine);
		taaOldColorAttach.destroy_attachment(engine);
		vkDestroyFramebuffer(engine->_device, taaFramebuffer, nullptr);
	}

	void TemporalFilter::prepare_jittering_data()
	{
		for (int i = 0; i != 36; ++i)
		{
			jittering.haltonSequence[i] = glm::vec4(create_halton_sequence(i + 1, 2), create_halton_sequence(i + 1, 3), 0.0f, 0.0f);
		}
	}

	float TemporalFilter::create_halton_sequence(int32_t index, int32_t base)
	{
		float f = 1;
		float r = 0;
		int32_t current = index;
		do
		{
			f = f / base;
			r = r + f * (current % base);
			current = glm::floor(current / base);
		}
		while (current > 0);

		return r;
	}

	void Composite::init(VkRenderer* engine)
	{
		MaterialData materialData;
		materialData.baseTemplate = "Composite";
		engine->_materialSystem.build_material("Composite", materialData);
	}

	void Composite::cleanup(VkRenderer* engine)
	{
		colorAttach.destroy_attachment(engine);
		velocityAttach.destroy_attachment(engine);
		vkDestroyFramebuffer(engine->_device, framebuffer, nullptr);
	}
}
