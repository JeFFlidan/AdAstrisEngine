#include "vk_engine.h"
#include "asset_loader.h"
#include "glm/fwd.hpp"
#include "material_asset.h"
#include "mesh_asset.h"
#include "prefab_asset.h"
#include "vk_descriptors.h"
#include "vk_textures.h"
#include "vk_mesh.h"
#include "material_system.h"
#include "vk_shaders.h"
#include "vk_renderpass.h"
#include "logger.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <stdint.h>
#include <stdio.h>
#include <unordered_map>
#include <vector>
#include <vk_types.h>
#include <vk_initializers.h>

#include "VkBootstrap.h"
#include "vk_types.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>

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

#define VK_CHECK(x)													 \
	do																 \
	{																 \
		VkResult err = x;											 \
		if (err)													 \
		{															 \
			std::cout << "Detected Vulkan error" << err << std::endl;\
			abort();												 \
		}															 \
	} while (0)

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

void VulkanEngine::init()
{
	// We initialize SDL and create a window with it. 
	SDL_Init(SDL_INIT_VIDEO);
	//SDL_ShowCursor(SDL_DISABLE);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
	
	_window = SDL_CreateWindow(
		"Vulkan Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		_windowExtent.width,
		_windowExtent.height,
		window_flags
	);

	SDL_Delay(100);

	init_vulkan();
	init_engine_systems();
	init_swapchain();
	init_commands();
	// Render passes should be created before framebuffers because framebuffers are created for special render passes
	init_renderpasses();
	init_framebuffers();
	init_sync_structures();
	init_descriptors();
	parse_prefabs();
	init_scene();
	init_imgui();
	
	//everything went fine
	_isInitialized = true;
}

void VulkanEngine::cleanup()
{
	// I must delete objects in the reverse order of their creation
	if (_isInitialized) 
	{
		vkWaitForFences(_device, 1, &_frames[0]._renderFence, true, 1000000000);
		vkWaitForFences(_device, 1, &_frames[1]._renderFence, true, 1000000000);

		_mainDeletionQueue.flush();

		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyDevice(_device, nullptr);
		vkDestroyInstance(_instance, nullptr);

		SDL_DestroyWindow(_window);
	}
}

void VulkanEngine::init_vulkan()
{
	LOG_INFO("Start");
    //PATH.erase(PATH.find("/bin"), 4);
    
    LOG_INFO("Path is {}", _projectPath);
	
	vkb::InstanceBuilder builder;

	auto inst_ret = builder.set_app_name("Vulkan Engine")
		.request_validation_layers(true)
		.require_api_version(1, 1, 0)
		.use_default_debug_messenger()
		.build();

	vkb::Instance vkb_inst = inst_ret.value();
	_instance = vkb_inst.instance;
	_debug_messenger = vkb_inst.debug_messenger;
	LOG_INFO("Finished crearing instance");

	SDL_Vulkan_CreateSurface(_window, _instance, &_surface);

	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(_surface)
		.add_required_extension("VK_EXT_descriptor_indexing")
		.select()
		.value();

	LOG_INFO("Finished picking up physical device");

	vkb::DeviceBuilder deviceBuilder{physicalDevice};
	VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features = {};
	shader_draw_parameters_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
	shader_draw_parameters_features.pNext = nullptr;
	shader_draw_parameters_features.shaderDrawParameters = VK_TRUE;
	vkb::Device vkbDevice = deviceBuilder.add_pNext(&shader_draw_parameters_features).build().value();
	LOG_INFO("Finished picking up logical device");

	_device = vkbDevice.device;
	_chosenGPU = physicalDevice.physical_device;

	_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	_graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = _chosenGPU;
	allocatorInfo.device = _device;
	allocatorInfo.instance = _instance;
	vmaCreateAllocator(&allocatorInfo, &_allocator);
	LOG_INFO("Finished creating allocator");

	_gpuProperties = vkbDevice.physical_device.properties;
	LOG_INFO("The GPU has a minimum buffer alignment of {}", _gpuProperties.limits.minUniformBufferOffsetAlignment)
}

void VulkanEngine::init_engine_systems()
{
	_projectPath = fs::current_path().string();
	_projectPath.erase(_projectPath.find("\\bin"), 4);

	_descriptorAllocator.init(_device);
	_descriptorLayoutCache.init(_device);

	_materialSystem.init(this);
}

void VulkanEngine::init_swapchain()
{
	vkb::SwapchainBuilder swapchainBuilder{_chosenGPU, _device, _surface};
	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.use_default_format_selection()
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(_windowExtent.width, _windowExtent.height)
		.build()
		.value();

	_swapchain = vkbSwapchain.swapchain;
	_swapchainImages = vkbSwapchain.get_images().value();
	_swapchainImageViews = vkbSwapchain.get_image_views().value();

	_swapchainImageFormat = vkbSwapchain.image_format;

	_mainDeletionQueue.push_function([=](){ vkDestroySwapchainKHR(_device, _swapchain, nullptr); });

	VkExtent3D imageExtent = {
		_windowExtent.width,
		_windowExtent.height,
		1
	 };

	// create_attachment(
	// 	_depthImage,
	// 	imageExtent,
	// 	VK_FORMAT_D32_SFLOAT,
	// 	VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	// 	VK_IMAGE_ASPECT_DEPTH_BIT
	// );

	create_attachment(
		_offscrColorImage, 
		imageExtent, 
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT);

	create_attachment(
		_offscrDepthImage,
		imageExtent, 
		VK_FORMAT_D32_SFLOAT,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT
	);

	_mainDeletionQueue.push_function([=](){
		// vkDestroyImageView(_device, _depthImage._imageView, nullptr);
		// vmaDestroyImage(_allocator, _depthImage._imageData._image, _depthImage._imageData._allocation);

		vkDestroyImageView(_device, _offscrDepthImage._imageView, nullptr);
		vmaDestroyImage(_allocator, _offscrDepthImage._imageData._image, _offscrDepthImage._imageData._allocation);

		vkDestroyImageView(_device, _offscrColorImage._imageView, nullptr);
		vmaDestroyImage(_allocator, _offscrColorImage._imageData._image, _offscrColorImage._imageData._allocation);		
	});
}

void VulkanEngine::init_imgui()
{
	VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	VkDescriptorPool imguiPool;
	VK_CHECK(vkCreateDescriptorPool(_device, &pool_info, nullptr, &imguiPool));

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForVulkan(_window);

	ImGui_ImplVulkan_InitInfo init_info{};
	init_info.Instance = _instance;
	init_info.PhysicalDevice = _chosenGPU;
	init_info.Device = _device;
	init_info.Queue = _graphicsQueue;
	init_info.DescriptorPool = imguiPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, _renderPass);

	immediate_submit([&](VkCommandBuffer cmd) {
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
	});

	ImGui_ImplVulkan_DestroyFontUploadObjects();

	_mainDeletionQueue.push_function([=](){
		vkDestroyDescriptorPool(_device, imguiPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
	});
}

void VulkanEngine::init_commands()
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

void VulkanEngine::init_renderpasses()
{
	vkutil::RenderPassBuilder::begin()
		.addColorAttachment(_offscrColorImage._format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.addDepthAttachment(_offscrDepthImage._format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL)
		.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
		.build(_device, _offscrRenderPass);
	
	vkutil::RenderPassBuilder::begin()
		.addColorAttachment(_swapchainImageFormat, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
		.build(_device, _renderPass);

	_mainDeletionQueue.push_function([=](){vkDestroyRenderPass(_device, _renderPass, nullptr);});
	_mainDeletionQueue.push_function([=](){vkDestroyRenderPass(_device, _offscrRenderPass, nullptr);});
}

void VulkanEngine::init_framebuffers()
{
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;

	fbInfo.attachmentCount = 1;
	fbInfo.width = _windowExtent.width;
	fbInfo.height = _windowExtent.height;
	fbInfo.layers = 1;

	const uint32_t swapchainImagecount = _swapchainImages.size();
	_framebuffers = std::vector<VkFramebuffer>(swapchainImagecount);

	_offscrFramebuffers = std::vector<VkFramebuffer>(swapchainImagecount);

	for (int i = 0; i != swapchainImagecount; ++i)
	{
		VkImageView attachments[2];
		attachments[0] = _swapchainImageViews[i];

		fbInfo.renderPass = _renderPass;
		fbInfo.pAttachments = &attachments[0];
		fbInfo.attachmentCount = 1;
		VK_CHECK(vkCreateFramebuffer(_device, &fbInfo, nullptr, &_framebuffers[i]));

		attachments[0] = _offscrColorImage._imageView;
		attachments[1] = _offscrDepthImage._imageView;

		fbInfo.renderPass = _offscrRenderPass;
		fbInfo.attachmentCount = 2;
		fbInfo.pAttachments = attachments;
		VK_CHECK(vkCreateFramebuffer(_device, &fbInfo, nullptr, &_offscrFramebuffers[i]));

		_mainDeletionQueue.push_function([=](){
			vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
			vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);

			vkDestroyFramebuffer(_device, _offscrFramebuffers[i], nullptr);
		});
	}
}

void VulkanEngine::init_sync_structures()
{
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

		_mainDeletionQueue.push_function([=](){
			vkDestroySemaphore(_device, _frames[i]._presentSemaphore, nullptr);
			vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);
		});
	}

	VkFenceCreateInfo uploadFenceCreateInfo = vkinit::fence_create_info();
	VK_CHECK(vkCreateFence(_device, &uploadFenceCreateInfo, nullptr, &_uploadContext._uploadFence));
	_mainDeletionQueue.push_function([=](){
		vkDestroyFence(_device, _uploadContext._uploadFence, nullptr);
	});
}

void VulkanEngine::init_descriptors()
{
	const size_t sceneParamBufferSize = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUSceneData));
	_sceneParameterBuffer = create_buffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	/*VkDescriptorSetLayoutBinding textureBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

	VkDescriptorSetLayoutCreateInfo set3info{};
	set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set3info.flags = 0;
	set3info.pNext = nullptr;
	set3info.bindingCount = 1;
	set3info.pBindings = &textureBind;

	vkCreateDescriptorSetLayout(_device, &set3info, nullptr, &_singleTextureSetLayout);
    
    vkCreateDescriptorSetLayout(_device, &set3info, nullptr, &_texturesSetLayout);*/

	//_descriptorAllocator.init(_device);
	//_descriptorLayoutCache.init(_device);

    VkDescriptorSetLayoutBinding bindingInfo;
	bindingInfo.descriptorCount = 3;
	bindingInfo.binding = 0;
	bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindingInfo.pImmutableSamplers = nullptr;
	bindingInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	
	VkDescriptorSetLayoutCreateInfo texturesSetInfo{};
	texturesSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	texturesSetInfo.bindingCount = 1;
	texturesSetInfo.pBindings = &bindingInfo;
	texturesSetInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

	const VkDescriptorBindingFlagsEXT flags =
		VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
		VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
		VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT |
		VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;

	VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
	bindingFlags.bindingCount = texturesSetInfo.bindingCount;
	bindingFlags.pBindingFlags = &flags;

    texturesSetInfo.pNext = &bindingFlags;
	
    VK_CHECK(vkCreateDescriptorSetLayout(_device, &texturesSetInfo, nullptr, &_texturesSetLayout));
	
	VkSamplerCreateInfo imgSamplerInfo = vkinit::sampler_create_info(VK_FILTER_NEAREST);
	VkSampler offscrColorSampler;
	vkCreateSampler(_device, &imgSamplerInfo, nullptr, &offscrColorSampler);

	for (int i = 0; i != FRAME_OVERLAP; ++i)
	{
		const int MAX_OBJECTS = 10000;
		_frames[i]._objectBuffer = create_buffer(sizeof(GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU); 

		_frames[i]._cameraBuffer = create_buffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		VkDescriptorBufferInfo cameraInfo;
		cameraInfo.buffer = _frames[i]._cameraBuffer._buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(GPUCameraData);

		VkDescriptorBufferInfo sceneInfo;
		sceneInfo.buffer = _sceneParameterBuffer._buffer;
		sceneInfo.offset = 0;
		sceneInfo.range = sizeof(GPUSceneData);

		VkDescriptorBufferInfo objectBufferInfo;
		objectBufferInfo.buffer = _frames[i]._objectBuffer._buffer;
		objectBufferInfo.offset = 0;
		objectBufferInfo.range = VK_WHOLE_SIZE;

		VkDescriptorImageInfo offscrColorImgInfo;
		offscrColorImgInfo.sampler = offscrColorSampler;
		offscrColorImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		offscrColorImgInfo.imageView = _offscrColorImage._imageView;

		vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &_descriptorAllocator)
			.bind_buffer(0, &cameraInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.bind_buffer(1, &sceneInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.bind_image(2, &offscrColorImgInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build(_frames[i]._globalDescriptor, _globalSetLayout);

		vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &_descriptorAllocator)
			.bind_buffer(0, &objectBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build(_frames[i]._objectDescriptor, _objectSetLayout);
	}

	for (int i = 0; i != FRAME_OVERLAP; ++i)
	{
		_mainDeletionQueue.push_function([=](){
			vmaDestroyBuffer(_allocator, _frames[i]._objectBuffer._buffer, _frames[i]._objectBuffer._allocation);
			vmaDestroyBuffer(_allocator, _frames[i]._cameraBuffer._buffer, _frames[i]._cameraBuffer._allocation);
		});
	}

	_mainDeletionQueue.push_function([=](){
		vmaDestroyBuffer(_allocator, _sceneParameterBuffer._buffer, _sceneParameterBuffer._allocation);
	});

	_mainDeletionQueue.push_function([=](){
		vkDestroyDescriptorSetLayout(_device, _objectSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(_device, _globalSetLayout, nullptr);
		//vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
	});
}

void VulkanEngine::init_scene()
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

	vkutil::MaterialData materialData;
	materialData.baseTemplate = "Postprocessing";
	_materialSystem.build_material("Postprocessing", materialData);
	for (int i = 0; i != 2; ++i)
	{
		_frames[i]._indirectCommandsBuffer = create_buffer(
			1000 * sizeof(VkDrawIndexedIndirectCommand),
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |  VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY);
	}
	
	_mainDeletionQueue.push_function([=](){
		for (int i = 0; i != 2; ++i)
		{
			vmaDestroyBuffer(_allocator, _frames[i]._indirectCommandsBuffer._buffer, _frames[i]._indirectCommandsBuffer._allocation);
		}
	});
	
	std::vector<glm::vec3> translation = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -8.0f),
		glm::vec3(0.0f, 0.0f, -14.0f),
		glm::vec3(0.0f, 0.0f, -20.0f),
		glm::vec3(0.0f, 0.0f, -26.0f)
	};

	for (int i = 0; i != _meshAndMaterialNames.size(); ++i)
	{
		RenderObject temp;
		temp.material = _materialSystem.get_material(_meshAndMaterialNames[i].materialName);
		temp.mesh = &_meshes[_meshAndMaterialNames[i].meshName];
		temp.transformMatrix = glm::translate(glm::mat4(1.0f), translation[i]);
		_renderables.push_back(temp);
	}
}

void VulkanEngine::draw()
{
	//ImGui::Render();

	// Wait for GPU
	VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(_device, 1, &get_current_frame()._renderFence));

	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, get_current_frame()._presentSemaphore, nullptr, &swapchainImageIndex));
	VK_CHECK(vkResetCommandBuffer(get_current_frame()._mainCommandBuffer, 0));

	VkCommandBufferBeginInfo cmdBeginInfo{};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;

	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(get_current_frame()._mainCommandBuffer, &cmdBeginInfo));

	VkClearValue clearValue;
	float flash = abs(std::sin(_frameNumber / 120.0f));
	clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(_offscrRenderPass, _windowExtent, _offscrFramebuffers[swapchainImageIndex]);

	rpInfo.clearValueCount = 2;
	
	VkClearValue clearValues[] = { clearValue, depthClear };
	rpInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(get_current_frame()._mainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	draw_objects(get_current_frame()._mainCommandBuffer, _renderables.data(), _renderables.size());

	//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), get_current_frame()._mainCommandBuffer);

	vkCmdEndRenderPass(get_current_frame()._mainCommandBuffer);

	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarrier.image = _offscrColorImage._imageData._image;
	imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		get_current_frame()._mainCommandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

	rpInfo = vkinit::renderpass_begin_info(_renderPass, _windowExtent, _framebuffers[swapchainImageIndex]);
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(get_current_frame()._mainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	draw_output_quad(get_current_frame()._mainCommandBuffer);

	vkCmdEndRenderPass(get_current_frame()._mainCommandBuffer);

	VK_CHECK(vkEndCommandBuffer(get_current_frame()._mainCommandBuffer));

	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = nullptr;
	
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submit.pWaitDstStageMask = &waitStage;
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &get_current_frame()._presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &get_current_frame()._renderSemaphore;

	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &get_current_frame()._mainCommandBuffer;

	VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, get_current_frame()._renderFence));

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;

	presentInfo.pSwapchains = &_swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &get_current_frame()._renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;
	VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

	_frameNumber++;
}

void VulkanEngine::run()
{
	SDL_Event e;
	bool bQuit = false;

	Uint64 previous_ticks{};
	float elapsed_seconds{};

	float lastX = static_cast<float>(_windowExtent.width / 2.0f);
	float lastY = static_cast<float>(_windowExtent.height / 2.0f);
	bool firstMouse = true;

	SDL_SetRelativeMouseMode(SDL_TRUE);

	//main loop
	while (!bQuit)
	{
		const Uint64 currentTicks{SDL_GetPerformanceCounter()};
		const Uint64 delta{ currentTicks - previous_ticks };
		previous_ticks = currentTicks;
		elapsed_seconds = delta / static_cast<float>(SDL_GetPerformanceFrequency());

		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			ImGui_ImplSDL2_ProcessEvent(&e);

			//close the window when user alt-f4s or clicks the X button
			if (e.type == SDL_QUIT)
			{
				bQuit = true;
			}
			if (e.type == SDL_KEYDOWN)
			{
				SDL_PumpEvents();
				if (e.key.keysym.sym == SDLK_SPACE)
				{
					++_selectedShader;
					if (_selectedShader > 1)
					{
						_selectedShader = 0;
					}
				}

				SDL_PumpEvents();
				if (e.key.keysym.sym == SDLK_ESCAPE)
					bQuit = true;
			}

			if (e.type == SDL_MOUSEMOTION)
			{
				float xPos, yPos;

				SDL_PumpEvents();

				xPos = static_cast<float>(e.motion.xrel);
				yPos = static_cast<float>(e.motion.yrel);
				if (firstMouse)
				{
					xPos = 0.0f;
					yPos = 0.0f;
					firstMouse = false;
				}

				float xoffset = xPos - lastX;
				float yoffset = yPos - lastY;

				camera.ProcessMouseMovement(xPos, yPos);
			}
		}

		//SDL_SetRelativeMouseMode(SDL_TRUE);
		SDL_PumpEvents();
		const Uint8 *state = SDL_GetKeyboardState(nullptr);
		if (state[SDL_SCANCODE_W])
			camera.processKeyboard(FORWARD, elapsed_seconds);
		if (state[SDL_SCANCODE_S])
			camera.processKeyboard(BACKWARD, elapsed_seconds);
		if (state[SDL_SCANCODE_A])
			camera.processKeyboard(LEFT, elapsed_seconds);
		if (state[SDL_SCANCODE_D])
			camera.processKeyboard(RIGHT, elapsed_seconds);
		//SDL_SetRelativeMouseMode(SDL_FALSE);

		//ImGui_ImplVulkan_NewFrame();
		//ImGui_ImplSDL2_NewFrame(_window);

		//ImGui::NewFrame();

		//ImGui::ShowDemoWindow();
		
		draw();
	}
}

void VulkanEngine::parse_prefabs()
{
	_materialSystem.build_default_templates();
	std::string assetsPath = _projectPath + "/assets/";
	fs::path direcory{ assetsPath };
	
	std::vector<Mesh> meshes;
	
	VkSamplerCreateInfo samplerInfo = vkinit::sampler_create_info(VK_FILTER_NEAREST);
	VkSampler sampler;
	VK_CHECK(vkCreateSampler(_device, &samplerInfo, nullptr, &sampler));

	std::vector<VkDescriptorImageInfo> baseColorImageInfos;
	std::vector<VkDescriptorImageInfo> normalImageInfos;
	std::vector<VkDescriptorImageInfo> armImageInfos;

	for (auto& p : fs::directory_iterator(direcory))
	{
		if (p.path().extension() == ".pref")
		{
			assets::AssetFile file;
			assets::load_binaryFile(p.path().string().c_str(), file);
			assets::PrefabInfo info = assets::read_prefab_info(&file);

			Mesh tempMesh{};
			tempMesh.load_from_mesh_asset(info.meshPath.c_str());
			meshes.push_back(tempMesh);
			std::string meshName = info.meshPath;
			meshName.erase(0, meshName.find_last_of("/"));
		    meshName.erase(meshName.find(".mesh"), 5);
		    meshName.erase(meshName.find('/'), 1);
			_meshes[meshName] = tempMesh;

		    assets::load_binaryFile(info.materialPath.c_str(), file);
		    assets::MaterialInfo materialInfo = assets::read_material_info(&file);
			vkutil::MaterialData materialData;
			materialData.baseTemplate = materialInfo.baseEffect;

			if (!materialInfo.textures.empty())
			{
				for (auto& tex : materialInfo.textures)
				{
					Texture tempTexture;
					vkutil::load_image_from_asset(*this, tex.second.c_str(), tempTexture.image);
					VkImageViewCreateInfo imageViewInfo = vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_UNORM, tempTexture.image._image, VK_IMAGE_ASPECT_COLOR_BIT);
					vkCreateImageView(_device, &imageViewInfo, nullptr, &tempTexture.imageView);

					if (tex.first == "texture_base_color")
					{
						_baseColorTextures.push_back(tempTexture);
						materialData.textures.push_back({ sampler, _baseColorTextures.back().imageView });

						VkDescriptorImageInfo tempInfo;
						tempInfo.imageView = _baseColorTextures.back().imageView;
						tempInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						tempInfo.sampler = sampler;
						baseColorImageInfos.push_back(tempInfo);
					}
					else if (tex.first == "normal_texture")
					{
						_normalTextures.push_back(tempTexture);
						materialData.textures.push_back({ sampler, _normalTextures.back().imageView });

						VkDescriptorImageInfo tempInfo;
						tempInfo.imageView = _normalTextures.back().imageView;
						tempInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						tempInfo.sampler = sampler;
					    normalImageInfos.push_back(tempInfo);
					}					
					else if (tex.first == "arm_texture")
					{
						_armTextures.push_back(tempTexture);
						materialData.textures.push_back({ sampler, _armTextures.back().imageView });

						VkDescriptorImageInfo tempInfo;
						tempInfo.imageView = _armTextures.back().imageView;
						tempInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						tempInfo.sampler = sampler;
					    armImageInfos.push_back(tempInfo);
					}
				}
			}
			else
			{
				LOG_WARNING("No textures in {} material", materialInfo.materialName);
			}

			
			_materialSystem.build_material(materialInfo.materialName, materialData);
			_meshAndMaterialNames.push_back({ meshName, materialInfo.materialName });			
		}
	}

	allocate_global_vertex_and_index_buffer(meshes);

	for (int i = 0; i != FRAME_OVERLAP; ++i)
	{		
		vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &_descriptorAllocator)
			.bind_image(0, baseColorImageInfos.data(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, baseColorImageInfos.size())
			.build_non_uniform(_frames[i]._texturesDescriptor, _texturesSetLayout, baseColorImageInfos.size());
	}
}

void VulkanEngine::allocate_global_vertex_and_index_buffer(std::vector<Mesh> meshes)
{
	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i != meshes.size(); ++i)
	{
		_globalVertexBufferSize += meshes[i]._vertices.size();
		_globalIndexBufferSize += meshes[i]._indices.size();
	}

	std::vector<assets::Vertex_f32_PNCV> vertices(_globalVertexBufferSize);
	std::vector<uint32_t> indices(_globalIndexBufferSize);

	size_t previousVerticesSize = 0;
	size_t previousIndicesSize = 0;

	for (int i = 0; i != meshes.size(); ++i)
	{
		memcpy(vertices.data() + previousVerticesSize, meshes[i]._vertices.data(), meshes[i]._vertices.size() * sizeof(assets::Vertex_f32_PNCV));
	    previousVerticesSize += meshes[i]._vertices.size();

	    memcpy(indices.data() + previousIndicesSize, meshes[i]._indices.data(), meshes[i]._indices.size() * sizeof(uint32_t));
	    previousIndicesSize += meshes[i]._indices.size();
	}

	AllocatedBuffer vertexStagingBuffer = create_buffer(_globalVertexBufferSize * sizeof(assets::Vertex_f32_PNCV),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	
	AllocatedBuffer indexStagingBuffer = create_buffer(_globalIndexBufferSize * sizeof(uint32_t),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	void* data;
	vmaMapMemory(_allocator, vertexStagingBuffer._allocation, &data);
	memcpy(data, vertices.data(), vertices.size() * sizeof(assets::Vertex_f32_PNCV));
	vmaUnmapMemory(_allocator, vertexStagingBuffer._allocation);

	data = nullptr;
	vmaMapMemory(_allocator, indexStagingBuffer._allocation, &data);
	memcpy(data, indices.data(), indices.size() * sizeof(uint32_t));
	vmaUnmapMemory(_allocator, indexStagingBuffer._allocation);

	_globalVertexBuffer = create_buffer(_globalVertexBufferSize * sizeof(assets::Vertex_f32_PNCV),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);

	_globalIndexBuffer = create_buffer(_globalIndexBufferSize * sizeof(assets::Vertex_f32_PNCV),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);

	immediate_submit([=](VkCommandBuffer cmd){
		VkBufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = vertices.size() * sizeof(assets::Vertex_f32_PNCV);
		vkCmdCopyBuffer(cmd, vertexStagingBuffer._buffer, _globalVertexBuffer._buffer, 1, &copy);

		copy.size = indices.size() * sizeof(uint32_t);
		vkCmdCopyBuffer(cmd, indexStagingBuffer._buffer, _globalIndexBuffer._buffer, 1, &copy);
	});
	
	_mainDeletionQueue.push_function([=](){
		vmaDestroyBuffer(_allocator, _globalVertexBuffer._buffer, _globalVertexBuffer._allocation);
		vmaDestroyBuffer(_allocator, _globalIndexBuffer._buffer, _globalIndexBuffer._allocation);
	});
	
	vmaDestroyBuffer(_allocator, vertexStagingBuffer._buffer, vertexStagingBuffer._allocation);
	vmaDestroyBuffer(_allocator, indexStagingBuffer._buffer, indexStagingBuffer._allocation);

	auto end = std::chrono::high_resolution_clock::now();

	auto diff = end - start;

	LOG_INFO("allocating mesh buffers took {} {}", std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / 1000000.0, "ms");
}

Mesh* VulkanEngine::get_mesh(const std::string& name)
{
	auto it = _meshes.find(name);
	if (it == _meshes.end())
	{
		return nullptr;
	}
	else
	{
		return &(*it).second;
	}
}

void VulkanEngine::draw_objects(VkCommandBuffer cmd, RenderObject* objects, int count)
{
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.f);
	projection[1][1] *= -1;			// Why?
 
	GPUCameraData camData;
	camData.view = view;
	camData.proj = projection;
	camData.viewproj = projection * view;

	void* data;
	vmaMapMemory(_allocator, get_current_frame()._cameraBuffer._allocation, &data);			// Get camera buffer's pointer
	memcpy(data, &camData, sizeof(GPUCameraData));
	vmaUnmapMemory(_allocator, get_current_frame()._cameraBuffer._allocation);

	float framed = (_frameNumber / 120.0f);

	_sceneParameters.ambientColor = { std::sin(framed), 0, std::cos(framed), 1 };

	char* sceneData;
	vmaMapMemory(_allocator, _sceneParameterBuffer._allocation, (void**)&sceneData);

	int frameIndex = _frameNumber % FRAME_OVERLAP;

	sceneData += pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;
	memcpy(sceneData, &_sceneParameters, sizeof(GPUSceneData));
	
	vmaUnmapMemory(_allocator, _sceneParameterBuffer._allocation);

	void* objectData;
	vmaMapMemory(_allocator, get_current_frame()._objectBuffer._allocation, &objectData);
	GPUObjectData* objectSSBO = (GPUObjectData*)objectData;

	std::vector<GPUObjectData> gpuObjectData;

	for (int i = 0; i != count; ++i)
	{
		RenderObject& object = objects[i];
		GPUObjectData temp;
		temp.color = object.color;
		temp.model = object.transformMatrix;
		gpuObjectData.push_back(temp);
	}

	memcpy(objectData, gpuObjectData.data(), gpuObjectData.size() * sizeof(GPUObjectData));

	vmaUnmapMemory(_allocator, get_current_frame()._objectBuffer._allocation);

	std::vector<IndirectBatch> draws = compact_draws(objects, count);

	AllocatedBuffer stagingBuffer = create_buffer(
		1000 * sizeof(VkDrawIndexedIndirectCommand),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY
	);

	data = nullptr;
	vmaMapMemory(_allocator, stagingBuffer._allocation, &data);
	
	VkDrawIndexedIndirectCommand* indexedDrawCommands = reinterpret_cast<VkDrawIndexedIndirectCommand*>(data);
	uint32_t vertexOffset = 0;

	uint32_t totalIndices = 0;
	
	for (int i = 0; i != count; ++i)
	{
		RenderObject& object = objects[i];
		indexedDrawCommands[i].indexCount = object.mesh->_indices.size();
		indexedDrawCommands[i].firstInstance = i;
		indexedDrawCommands[i].instanceCount = 1;
		indexedDrawCommands[i].vertexOffset = vertexOffset;
		indexedDrawCommands[i].firstIndex = totalIndices;
		vertexOffset += object.mesh->_vertices.size();
		totalIndices += object.mesh->_indices.size();
	}

	vmaUnmapMemory(_allocator, stagingBuffer._allocation);

	immediate_submit([=](VkCommandBuffer cmd){
		VkBufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = 1000 * sizeof(VkDrawIndexedIndirectCommand);
		vkCmdCopyBuffer(cmd, stagingBuffer._buffer, get_current_frame()._indirectCommandsBuffer._buffer, 1, &copy);
	});

	vmaDestroyBuffer(_allocator, stagingBuffer._buffer, stagingBuffer._allocation);

	//bind_material(cmd, draws[0].material);
	VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &_globalVertexBuffer._buffer, &offset);
    vkCmdBindIndexBuffer(cmd, _globalIndexBuffer._buffer, offset, VK_INDEX_TYPE_UINT32);
	
	for (IndirectBatch& draw : draws)
	{
  		bind_material(cmd, draw.material);
		//bind_material(cmd, draw.material);
		//bind_mesh(cmd, draw.mesh);

		VkDeviceSize indirectOffset = draw.first * sizeof(VkDrawIndexedIndirectCommand);
		uint32_t drawStride = sizeof(VkDrawIndexedIndirectCommand);

		//vkCmdDraw(cmd, object.mesh->_vertices.size(), 1, 0, i);
		//vkCmdDrawIndexed(cmd, static_cast<uint32_t>(object.mesh->_indices.size()), 1, 0, 0, i);
		//vkCmdDrawIndirect(cmd, get_current_frame()._indirectCommandsBuffer._buffer, indirectOffset, draw.count, drawStride);

		vkCmdDrawIndexedIndirect(cmd, get_current_frame()._indirectCommandsBuffer._buffer, indirectOffset, draw.count, drawStride);
	}
}

void VulkanEngine::draw_output_quad(VkCommandBuffer cmd)
{
	int frameIndex = _frameNumber % FRAME_OVERLAP;

	auto material = _materialSystem.get_material("Postprocessing");
	if (material != nullptr)
	{
		auto meshPass = material->original->passShaders[vkutil::MeshpassType::Forward];
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, meshPass->pipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, meshPass->layout, 0, 1, &get_current_frame()._globalDescriptor, 0, nullptr);
	}
	else
	{
		LOG_FATAL("Invalid material for output quad, draw_output_quad func");
	}

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &_outputQuad._vertexBuffer._buffer, &offset);
	vkCmdDraw(cmd, _outputQuad._vertices.size(), 1, 0, 0);
}

FrameData& VulkanEngine::get_current_frame()
{
	return _frames[_frameNumber % FRAME_OVERLAP];
}

size_t VulkanEngine::pad_uniform_buffer_size(size_t originalSize)
{
	size_t minUboAlignment = _gpuProperties.limits.minUniformBufferOffsetAlignment;
	size_t alignedSize = originalSize;
	if (minUboAlignment > 0)
	{
		alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return alignedSize;
}

void VulkanEngine::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function)
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

void VulkanEngine::create_attachment(
			Attachment& attachment, 
			VkExtent3D imageExtent, 
			VkFormat format, 
			VkImageUsageFlags usageFlags, 
			VkImageAspectFlags aspectFlags)
{
	attachment._format = format;

	VkImageCreateInfo img_info = vkinit::image_create_info(attachment._format, usageFlags, imageExtent);
	
	VmaAllocationCreateInfo img_allocinfo{};
	img_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	img_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);	// VMA allocate the image into VRAM in all situations

	vmaCreateImage(_allocator, &img_info, &img_allocinfo, &attachment._imageData._image, &attachment._imageData._allocation, nullptr);

	VkImageViewCreateInfo view_info = vkinit::imageview_create_info(attachment._format, attachment._imageData._image, aspectFlags);
	VK_CHECK(vkCreateImageView(_device, &view_info, nullptr, &attachment._imageView));
}

AllocatedBuffer VulkanEngine::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
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

std::vector<IndirectBatch> VulkanEngine::compact_draws(RenderObject* objects, int count)
{
	std::vector<IndirectBatch> draws;
	IndirectBatch firstDraw;
	firstDraw.mesh = objects[0].mesh;
	firstDraw.material = objects[0].material;
	firstDraw.first = 0;
	firstDraw.count = 1;

	draws.push_back(firstDraw);

	for (int i = 1; i < count; ++i)
	{
		bool sameMesh = objects[i].mesh == draws.back().mesh;
		bool sameMaterial = objects[i].material == draws.back().material;

		if (sameMesh && sameMaterial)
			draws.back().count++;
		else
		{
			IndirectBatch newDraw;
			newDraw.mesh = objects[i].mesh;
			newDraw.material = objects[i].material;
			newDraw.first = i;
			newDraw.count = 1;

			draws.push_back(newDraw);
		}
	}

	return draws;
}

void VulkanEngine::bind_material(VkCommandBuffer cmd, vkutil::Material* material)
{
	auto renderPass = material->original->passShaders[vkutil::MeshpassType::Forward];
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->layout, 0, 1, &get_current_frame()._globalDescriptor, 0, nullptr);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->layout, 1, 1, &get_current_frame()._objectDescriptor, 0, nullptr);

	if (get_current_frame()._texturesDescriptor != VK_NULL_HANDLE)
	{
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->layout, 2, 1, &get_current_frame()._texturesDescriptor, 0, nullptr);
	}
}

void VulkanEngine::bind_mesh(VkCommandBuffer cmd, Mesh* mesh)
{
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &mesh->_vertexBuffer._buffer, &offset);
	vkCmdBindIndexBuffer(cmd, mesh->_indexBuffer._buffer, offset, VK_INDEX_TYPE_UINT32);
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

