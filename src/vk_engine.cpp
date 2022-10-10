#include "vk_engine.h"
#include "glm/fwd.hpp"
#include "mesh_asset.h"
#include "vk_descriptors.h"
#include "vk_textures.h"
#include "vk_mesh.h"
#include "material_system.h"
#include "vk_shaders.h"
#include "logger.h"
//#include "material_asset.h"
//#include "prefab_asset.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <vk_types.h>
#include <vk_initializers.h>

#include "VkBootstrap.h"
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
#include <fstream>
#include <chrono>

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
	init_swapchain();
	init_commands();
	// Render passes should be created before framebuffers because framebuffers are created for special render passes
	init_default_renderpass();
	init_offscreen_renderpass();
	init_framebuffers();
	init_sync_structures();
	init_descriptors();
	init_pipelines();
	load_images();
	load_meshes();
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

void VulkanEngine::init_default_renderpass()
{
	// Set up the description of the image we will be writing into with rendering commands
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = _swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// after the renderpass ends, the image has to be able on a layout ready for display with the swapchain
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
	//Define color attachment for subpass
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// VkAttachmentDescription depthAttachment{};
	// depthAttachment.flags = 0;
	// depthAttachment.format = _depthImage._format;
	// depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	// depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// VkAttachmentReference depthAttachmentRef{};
	// depthAttachmentRef.attachment = 1;
	// depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	// subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// VkSubpassDependency depthDependency{};
	// depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	// depthDependency.dstSubpass = 0;
	// depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	// depthDependency.srcAccessMask = 0;
	// depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	// depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	// VkAttachmentDescription attachmnets[2] = { colorAttachment, depthAttachment };
	// VkSubpassDependency dependencies[2] = { dependency, depthDependency };

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VK_CHECK(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass));

	_mainDeletionQueue.push_function([=](){vkDestroyRenderPass(_device, _renderPass, nullptr);});
}

void VulkanEngine::init_offscreen_renderpass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = _offscrColorImage._format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.flags = 0;
	depthAttachment.format = _offscrDepthImage._format;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency depthDependency{};
	depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depthDependency.dstSubpass = 0;
	depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depthDependency.srcAccessMask = 0;
	depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachmnets[2] = { colorAttachment, depthAttachment };
	VkSubpassDependency dependencies[2] = { dependency, depthDependency };

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachmnets;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies;

	VK_CHECK(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_offscrRenderPass));

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

bool VulkanEngine::load_shader_module(const char* filePath, VkShaderModule* outShaderModule)
{
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		return false;

	size_t fileSize = static_cast<size_t>(file.tellg());

	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	file.seekg(0);

	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

	file.close();

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;

	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		return false;

	*outShaderModule = shaderModule;
	return true;
}

void VulkanEngine::init_descriptors()
{
	const size_t sceneParamBufferSize = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUSceneData));
	_sceneParameterBuffer = create_buffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	VkDescriptorSetLayoutBinding textureBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

	VkDescriptorSetLayoutCreateInfo set3info{};
	set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set3info.flags = 0;
	set3info.pNext = nullptr;
	set3info.bindingCount = 1;
	set3info.pBindings = &textureBind;

	vkCreateDescriptorSetLayout(_device, &set3info, nullptr, &_singleTextureSetLayout);
    
    vkCreateDescriptorSetLayout(_device, &set3info, nullptr, &_texturesSetLayout);

	_descriptorAllocator.init(_device);
	_descriptorLayoutCache.init(_device);
	
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

void VulkanEngine::init_pipelines()
{
	VkShaderModule meshVertShader;
	if (!load_shader_module("E:/VulkanEngine/shaders/tri_mesh.vert.spv", &meshVertShader))
	{
		LOG_ERROR("Error when building the mesh triangle vertex shader module");
	}
	else
	{
		LOG_SUCCESS("Mesh triangle fragment shader successfully loaded");
	}

	VkShaderModule coloredShader;
	if (!load_shader_module("E:/VulkanEngine/shaders/colored_triangle.frag.spv", &coloredShader))
	{
		LOG_ERROR("Error when building the mesh triangle vertex shader module");
	}
	else
	{
		LOG_SUCCESS("Colored triangle shader successfully loaded");
	}

	VkShaderModule texturedMeshShader;
	if (!load_shader_module("E:/VulkanEngine/shaders/textured_lit.frag.spv", &texturedMeshShader))
	{
		LOG_ERROR("Error when building the textured mesh fragment shader module");
	}
	else
	{
		LOG_SUCCESS("Textured mesh fragment shader successfully loaded");
	}

	VkShaderModule outputQuadVertShader;
	if (!load_shader_module("E:/VulkanEngine/shaders/output_quad.vert.spv", &outputQuadVertShader))
	{
		LOG_ERROR("Error when building the textured mesh fragment shader module");
	}
	else
	{
		LOG_SUCCESS("Textured mesh fragment shader successfully loaded");
	}

	VkShaderModule outputQuadFragShader;
	if (!load_shader_module("E:/VulkanEngine/shaders/output_quad.frag.spv", &outputQuadFragShader))
	{
		LOG_ERROR("Error when building the textured mesh fragment shader module");
	}
	else
	{
		LOG_SUCCESS("Textured mesh fragment shader successfully loaded");
	}

	vkutil::Shader firstShader(_device);
	firstShader.load_shader_module("E:/VulkanEngine/shaders/tri_mesh.vert.spv");
	firstShader.spv_reflect_test();

	vkutil::PipelineBuilder pipelineBuilder;

	pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();
	pipelineBuilder._inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	
	pipelineBuilder._viewport.x = 0.0f;
	pipelineBuilder._viewport.y = 0.0f;
	pipelineBuilder._viewport.width = static_cast<float>(_windowExtent.width);
	pipelineBuilder._viewport.height = static_cast<float>(_windowExtent.height);
	pipelineBuilder._viewport.minDepth = 0.0f;
	pipelineBuilder._viewport.maxDepth = 1.0f;

	pipelineBuilder._scissor.offset = { 0, 0 };
	pipelineBuilder._scissor.extent = _windowExtent;

	pipelineBuilder._depthStencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
	pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);
	pipelineBuilder._multisampling = vkinit::multisampling_state_create_info();
	pipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();

	VertexInputDescription vertexDescription = Mesh::get_vertex_description();

	pipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	pipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

	pipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	pipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	VkPipelineLayoutCreateInfo texturedPipelineLayoutInfo = vkinit::pipeline_layout_create_info();

	VkPushConstantRange pushConstant;
	pushConstant.offset = 0;
	pushConstant.size = sizeof(MeshPushConstants);
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	texturedPipelineLayoutInfo.pPushConstantRanges = &pushConstant;
	texturedPipelineLayoutInfo.pushConstantRangeCount = 1;

	VkDescriptorSetLayout texturedSetLayouts[] = { _globalSetLayout, _objectSetLayout, _singleTextureSetLayout };

	texturedPipelineLayoutInfo.setLayoutCount = 3;
	texturedPipelineLayoutInfo.pSetLayouts = texturedSetLayouts;

	VkPipelineLayout texturedPipelineLayout;
	VK_CHECK(vkCreatePipelineLayout(_device, &texturedPipelineLayoutInfo, nullptr, &texturedPipelineLayout));

	pipelineBuilder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));
	pipelineBuilder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, texturedMeshShader));

	pipelineBuilder._pipelineLayout = texturedPipelineLayout;

	VkPipeline texPipeline = pipelineBuilder.build_pipeline(_device, _offscrRenderPass);
	create_material(texPipeline, texturedPipelineLayout, "texturedmesh");
	
	pipelineBuilder._shaderStages.clear();
	pipelineBuilder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));
	pipelineBuilder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, coloredShader));

	VkPipeline coloredPipeline = pipelineBuilder.build_pipeline(_device, _offscrRenderPass);
	create_material(coloredPipeline, texturedPipelineLayout, "colored_mesh");

	VkPipelineLayoutCreateInfo outputQuadPipelineLayoutInfo = vkinit::pipeline_layout_create_info();
	outputQuadPipelineLayoutInfo.setLayoutCount = 1;
	outputQuadPipelineLayoutInfo.pSetLayouts = &_globalSetLayout;

	VkPipelineLayout outputQuadLayout;
	VK_CHECK(vkCreatePipelineLayout(_device, &outputQuadPipelineLayoutInfo, nullptr, &outputQuadLayout));

	pipelineBuilder._pipelineLayout = outputQuadLayout;

	pipelineBuilder._depthStencil = vkinit::depth_stencil_create_info(false, false, VK_COMPARE_OP_LESS_OR_EQUAL);

	VertexInputDescription outputQuadVertDescription = Plane::get_vertex_description();

	pipelineBuilder._shaderStages.clear();
	pipelineBuilder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, outputQuadVertShader));
	pipelineBuilder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, outputQuadFragShader));

	pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();
	pipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = outputQuadVertDescription.bindings.size();
	pipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = outputQuadVertDescription.bindings.data();
	pipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = outputQuadVertDescription.attributes.size();
	pipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = outputQuadVertDescription.attributes.data();

	VkPipeline outputQuadPipeline = pipelineBuilder.build_pipeline(_device, _renderPass);
	create_material(outputQuadPipeline, outputQuadLayout, "outputquad");	

	vkDestroyShaderModule(_device, meshVertShader, nullptr);
	vkDestroyShaderModule(_device, texturedMeshShader, nullptr);
	vkDestroyShaderModule(_device, outputQuadVertShader, nullptr);
	vkDestroyShaderModule(_device, outputQuadFragShader, nullptr);

	_mainDeletionQueue.push_function([=](){
		vkDestroyPipeline(_device, outputQuadPipeline, nullptr);
		vkDestroyPipeline(_device, texPipeline, nullptr);

		vkDestroyPipelineLayout(_device, outputQuadLayout, nullptr);
		vkDestroyPipelineLayout(_device, texturedPipelineLayout, nullptr);
	});
}

void VulkanEngine::init_scene()
{
	RenderObject map;
	map.mesh = get_mesh("empire");
	map.material = get_material("texturedmesh");
	map.transformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, -10.0f, 0.0f));

	//_renderables.push_back(map);

	VkSamplerCreateInfo samplerInfo = vkinit::sampler_create_info(VK_FILTER_NEAREST);

	VkSampler blockySampler;
	vkCreateSampler(_device, &samplerInfo, nullptr, &blockySampler);

	Material* texturedMat = get_material("texturedmesh");

	VkDescriptorImageInfo imageBufferInfo{};
	imageBufferInfo.sampler = blockySampler;
	imageBufferInfo.imageView = _loadedTextures["empire_diffuse"].imageView;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &_descriptorAllocator)
		.bind_image(0, &imageBufferInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build(texturedMat->textureSet, _singleTextureSetLayout);

	VkWriteDescriptorSet texture1 = vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texturedMat->textureSet, &imageBufferInfo, 0);

	vkUpdateDescriptorSets(_device, 1, &texture1, 0, nullptr);

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

	
	std::vector<std::string> meshNames  = {
 		"door",
		"gun",
		"gun2",
		"monkey_flat",
		"monkey_smooth"
	};

	std::vector<glm::vec3> translation = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -8.0f),
		glm::vec3(0.0f, 0.0f, -14.0f),
		glm::vec3(0.0f, 0.0f, -20.0f),
		glm::vec3(0.0f, 0.0f, -26.0f)
	};
	
	for (int i = 0; i != 5; ++i)
	{
		RenderObject temp;
		temp.mesh = &_meshes[meshNames[i]];
		temp.material = get_material("colored_mesh");
		temp.transformMatrix = glm::translate(glm::mat4(1.0f), translation[i]);
		_renderables.push_back(temp);
	}

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

void VulkanEngine::load_images()
{
	Texture lostEmpire;

	vkutil::load_image_from_asset(*this, "E:/VulkanEngine/empireAssets/lost_empire-RGBA.tx", lostEmpire.image);

	VkImageViewCreateInfo imageInfo = vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_UNORM, lostEmpire.image._image, VK_IMAGE_ASPECT_COLOR_BIT);
	vkCreateImageView(_device, &imageInfo, nullptr, &lostEmpire.imageView);

	_loadedTextures["empire_diffuse"] = lostEmpire;

	std::vector<Texture> textures;
	std::vector<std::string> texturesNames = {
		"Gun_BaseColor",
		"Gun_2_BaseColor",
		"Sci fi door 1_BaseColor"
	};

	for (int i = 0; i != texturesNames.size(); ++i)
	{
		std::cout << "Texture " << i << std::endl;
		vkutil::load_image_from_asset(*this, ("E:/VulkanEngine/assets/" + texturesNames[i] + ".tx").c_str(), textures[i].image);
		std::cout << "Test 1" << std::endl;
		VkImageViewCreateInfo imageInfo = vkinit::imageview_create_info(VK_FORMAT_R8G8B8_UNORM, textures[i].image._image, VK_IMAGE_ASPECT_COLOR_BIT);
		std::cout << "Test 2 " << std::endl;
		vkCreateImageView(_device, &imageInfo, nullptr, &textures[i].imageView);

		_loadedTextures[texturesNames[i]] = textures[i];
	}

	// assets::MaterialInfo test;
	// test.baseEffect = "Test";
	// test.textures = { {"texture_base_color", "E:/VulkanEngine/empireAssets/lost_empire-RGBA.tx"} };
	// test.customProperties = { { "prop_test1", "test1" }, { "prop_test2", "test2" } };
	// test.mode = assets::MaterialMode::OPAQUE;
// 
	// assets::AssetFile file = assets::pack_material(&test);
	// assets::save_binaryFile("E:/VulkanEngine/empireAssets/material_test.mat", file);

	// assets::PrefabInfo test;
	// test.meshPath = "E:/VulkanEngine/empireAssets/lost_empire.mesh";
	// test.materialPath = "E:/VulkanEngine/empireAssets/material_test.mat";
	// std::array<float, 16> matrix;

	// glm::mat4 mtx = glm::mat4(1.0f);
	// const float* source = (const float*)glm::value_ptr(mtx);
	// memcpy(matrix.data(), (float*)source, sizeof(float) * 16);
	// for (int i = 0; i != 16; ++i)
	// {
	// 	matrix[i] = 1.0f;
	// }
	
	// test.matrix = matrix;

	// assets::AssetFile file = assets::pack_prefab(&test);
	// assets::save_binaryFile("E:/VulkanEngine/empireAssets/test_prefab.pref", file);
}

void VulkanEngine::load_meshes()
{
	Mesh lostEmpire{};

	std::vector<Mesh> meshes;
	
	auto start = std::chrono::high_resolution_clock::now();

	std::vector<std::string> meshNames  = {
 		"door",
		"gun",
		"gun2",
		"monkey_flat",
		"monkey_smooth"
	};
	
	for (auto& meshName : meshNames)
	{
		Mesh temp{};
		temp.load_from_mesh_asset(("E:/VulkanEngine/assets/" + meshName + ".mesh").c_str());
		meshes.push_back(temp);

		_meshes[meshName] = temp;
	}

	std::cout << meshes[2]._vertices.size() << std::endl;
	std::cout << meshes[3]._vertices.size() << std::endl;
	
	lostEmpire.load_from_mesh_asset("E:/VulkanEngine/empireAssets/lost_empire.mesh");

	auto end = std::chrono::high_resolution_clock::now();
	auto diff = end - start;

	LOG_INFO("reading mesh asset file took {} {}", std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / 1000000.0, "ms");

	allocate_global_vertex_and_index_buffer(meshes);

	upload_mesh(lostEmpire);

	_meshes["empire"] = lostEmpire;
}

void VulkanEngine::upload_mesh(Mesh& mesh)
{
	auto start = std::chrono::high_resolution_clock::now();

	size_t vertexBufferSize = mesh._vertices.size() * sizeof(assets::Vertex_f32_PNCV);
	size_t indexBufferSize = mesh._indices.size() * sizeof(uint32_t);

	AllocatedBuffer vertexStagingBuffer = create_buffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	AllocatedBuffer indexStagingBuffer = create_buffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	void* data;
	vmaMapMemory(_allocator, vertexStagingBuffer._allocation, &data);
	memcpy(data, mesh._vertices.data(), mesh._vertices.size() * sizeof(assets::Vertex_f32_PNCV));
	vmaUnmapMemory(_allocator, vertexStagingBuffer._allocation);

	data = nullptr;
	vmaMapMemory(_allocator, indexStagingBuffer._allocation, &data);
	memcpy(data, mesh._indices.data(), mesh._indices.size() * sizeof(uint32_t));
	vmaUnmapMemory(_allocator, indexStagingBuffer._allocation);

	mesh._vertexBuffer = create_buffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	mesh._indexBuffer = create_buffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	immediate_submit([=](VkCommandBuffer cmd){
		VkBufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = vertexBufferSize;
		vkCmdCopyBuffer(cmd, vertexStagingBuffer._buffer, mesh._vertexBuffer._buffer, 1, &copy);

		copy.size = indexBufferSize;
		vkCmdCopyBuffer(cmd, indexStagingBuffer._buffer, mesh._indexBuffer._buffer, 1, &copy);
	});

	_mainDeletionQueue.push_function([=](){
		vmaDestroyBuffer(_allocator, mesh._vertexBuffer._buffer, mesh._vertexBuffer._allocation);
		vmaDestroyBuffer(_allocator, mesh._indexBuffer._buffer, mesh._indexBuffer._allocation);
	});

	vmaDestroyBuffer(_allocator, vertexStagingBuffer._buffer, vertexStagingBuffer._allocation);
	vmaDestroyBuffer(_allocator, indexStagingBuffer._buffer, indexStagingBuffer._allocation);

	auto end = std::chrono::high_resolution_clock::now();

	auto diff = end - start;

	LOG_INFO("allocating mesh buffers took {} {}", std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / 1000000.0, "ms");
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
	std::cout << "Start writing to the vectors" << std::endl;
	for (int i = 0; i != meshes.size(); ++i)
	{
		memcpy(vertices.data() + previousVerticesSize, meshes[i]._vertices.data(), meshes[i]._vertices.size() * sizeof(assets::Vertex_f32_PNCV));
	    previousVerticesSize += meshes[i]._vertices.size();

	    memcpy(indices.data() + previousIndicesSize, meshes[i]._indices.data(), meshes[i]._indices.size() * sizeof(uint32_t));
	    previousIndicesSize += meshes[i]._indices.size();
	}

	std::cout << "Finish writing to the vectors" << std::endl;

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

Material* VulkanEngine::create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name)
{
	Material mat;
	mat.pipeline = pipeline;
	mat.pipelineLayout = layout;
	_materials[name] = mat;
	return &_materials[name];
}

Material* VulkanEngine::get_material(const std::string& name)
{
	auto it = _materials.find(name);
	if (it == _materials.end())
	{
		return nullptr;
	}
	else
	{
		return &(*it).second;
	}
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

	std::vector<GPUObjectData> colors;

	for (int i = 0; i != count; ++i)
	{
		RenderObject& object = objects[i];
		GPUObjectData temp;
		temp.color = object.color;
		temp.model = object.transformMatrix;
		colors.push_back(temp);
	}

	memcpy(objectData, colors.data(), colors.size() * sizeof(GPUObjectData));

	vmaUnmapMemory(_allocator, get_current_frame()._objectBuffer._allocation);

	std::vector<IndirectBatch> draws = compact_draws(objects, count);

	AllocatedBuffer tempBuffer = create_buffer(
		1000 * sizeof(VkDrawIndirectCommand),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY);

	data = nullptr;
	vmaMapMemory(_allocator, tempBuffer._allocation, &data);

	VkDrawIndirectCommand* drawCommands = reinterpret_cast<VkDrawIndirectCommand*>(data);

	for (int i = 0; i != count; ++i)
	{
		RenderObject& object = objects[i];
		drawCommands[i].vertexCount = objects->mesh->_vertices.size();
		drawCommands[i].instanceCount = 1;
		drawCommands[i].firstVertex = 0;
		drawCommands[i].firstInstance = i;
	}

	vmaUnmapMemory(_allocator, tempBuffer._allocation);

	/*immediate_submit([=](VkCommandBuffer cmd){
		VkBufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = 1000 * sizeof(VkDrawIndirectCommand);
		vkCmdCopyBuffer(cmd, tempBuffer._buffer, get_current_frame()._indirectCommandsBuffer._buffer, 1, &copy);
	});*/

	vmaDestroyBuffer(_allocator, tempBuffer._buffer, tempBuffer._allocation);

	AllocatedBuffer stagingBuffer = create_buffer(
		1000 * sizeof(VkDrawIndexedIndirectCommand),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY
	);

	data = nullptr;
	vmaMapMemory(_allocator, stagingBuffer._allocation, &data);
	
	VkDrawIndexedIndirectCommand* indexedDrawCommands = reinterpret_cast<VkDrawIndexedIndirectCommand*>(data);
	uint32_t vertexOffset = 0;
	
	for (int i = 0; i != count; ++i)
	{
		RenderObject& object = objects[i];
		indexedDrawCommands[i].indexCount = object.mesh->_indices.size();
		indexedDrawCommands[i].firstInstance = i;
		indexedDrawCommands[i].instanceCount = 1;
		indexedDrawCommands[i].vertexOffset = vertexOffset;
		indexedDrawCommands[i].firstIndex = 0;
		vertexOffset += object.mesh->_vertices.size();
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

	bind_material(cmd, draws[0].material);
	VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &_globalVertexBuffer._buffer, &offset);
    vkCmdBindIndexBuffer(cmd, _globalIndexBuffer._buffer, offset, VK_INDEX_TYPE_UINT32);
	
	for (IndirectBatch& draw : draws)
	{
		//bind_material(cmd, draw.material);
		//bind_mesh(cmd, draw.mesh);

		glm::mat4 model = glm::mat4(1.0f);
		
		MeshPushConstants constants;
		constants.render_matrix = model;
		constants.data = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

		vkCmdPushConstants(cmd, draw.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

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

	auto material = &_materials["outputquad"];
	if (material != nullptr)
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 0, 1, &get_current_frame()._globalDescriptor, 0, nullptr);
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

void VulkanEngine::bind_material(VkCommandBuffer cmd, Material* material)
{
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 0, 1, &get_current_frame()._globalDescriptor, 0, nullptr);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 1, 1, &get_current_frame()._objectDescriptor, 0, nullptr);

	if (material->textureSet != VK_NULL_HANDLE)
	{
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 2, 1, &material->textureSet, 0, nullptr);
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

