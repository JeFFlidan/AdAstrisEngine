#include "vulkan_ui_window_backend.h"
#include "vulkan_common.h"
#include "vulkan_queue.h"
#include "ui_core/font_manager.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_win32.h"
#include <stb_image/stb_image.h>

using namespace ad_astris;
using namespace vulkan;

PFN_vkVoidFunction load_function(const char* functionName, void* userData)
{
	VulkanRHI* rhi = static_cast<VulkanRHI*>(userData);
	PFN_vkVoidFunction instanceAddr = vkGetInstanceProcAddr(rhi->get_instance()->get_handle(), functionName);
	PFN_vkVoidFunction deviceAddr = vkGetDeviceProcAddr(rhi->get_device()->get_device(), functionName);
	return deviceAddr ? deviceAddr : instanceAddr;
}

void VulkanUIWindowBackend::init(rhi::UIWindowBackendInitContext& initContext, rhi::Sampler sampler)
{
	_rhi = static_cast<VulkanRHI*>(initContext.rhi);
	_mainWindow = initContext.window;
	_fileSystem = initContext.fileSystem;
	
	VkDescriptorPoolSize poolSizes[] = {
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

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = std::size(poolSizes);
	poolInfo.pPoolSizes = poolSizes;
	VK_CHECK(vkCreateDescriptorPool(_rhi->get_device()->get_device(), &poolInfo, nullptr, &_descriptorPool));

	ImGui::CreateContext();
	
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = _mainWindow->get_width();
	io.DisplaySize.y = _mainWindow->get_height();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = nullptr;

#ifdef _WIN32
	ImGui_ImplWin32_LoadFunctions(load_function, _rhi);
	ImGui_ImplWin32_InitForVulkan(_mainWindow->get_hWnd());
#endif
	
	ImGui_ImplVulkan_InitInfo vulkanInitInfo{};
	vulkanInitInfo.Instance = _rhi->get_instance()->get_handle();
	vulkanInitInfo.PhysicalDevice = _rhi->get_device()->get_physical_device();
	vulkanInitInfo.Device = _rhi->get_device()->get_device();
	vulkanInitInfo.Queue = _rhi->get_device()->get_graphics_queue()->get_handle();
	vulkanInitInfo.DescriptorPool = _descriptorPool;
	vulkanInitInfo.MinImageCount = 3;
	vulkanInitInfo.ImageCount = 3;
	vulkanInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	vulkanInitInfo.UseDynamicRendering = true;
	vulkanInitInfo.ColorAttachmentFormat = _rhi->get_swap_chain()->get_format();

	ImGui_ImplVulkan_LoadFunctions(load_function, _rhi);
	ImGui_ImplVulkan_Init(&vulkanInitInfo, VK_NULL_HANDLE);
	rhi::CommandBuffer cmdBuffer;

	//uicore::FontManager::load_default_fonts(initContext.fileSystem->get_engine_root_path().c_str());

	io.Fonts->AddFontDefault();
	_fontSize17 = io.Fonts->AddFontFromFileTTF((initContext.fileSystem->get_engine_root_path() + "/fonts/unispace bd.ttf").c_str(), 17.0f);
	_fontSize14 = io.Fonts->AddFontFromFileTTF((initContext.fileSystem->get_engine_root_path() + "/fonts/unispace bd.ttf").c_str(), 14.0f);
	
	_rhi->begin_command_buffer(&cmdBuffer, rhi::QueueType::GRAPHICS);
	ImGui_ImplVulkan_CreateFontsTexture(get_vk_obj(&cmdBuffer)->get_handle());
	
	std::vector<std::pair<io::URI, editor::IconType>> relativePathAndIconType = {
		{ "/icons/3Dmodel.png", editor::IconType::MODEL_FILE },
		{ "/icons/folder.png", editor::IconType::FOLDER },
		{ "/icons/image.png", editor::IconType::TEXTURE_FILE },
		{ "/icons/level.png", editor::IconType::LEVEL_FILE },
		{ "/icons/material.png", editor::IconType::MATERIAL_FILE }
	};

	std::vector<rhi::Buffer> cpuBuffers;
	for (auto& pair : relativePathAndIconType)
	{
		io::URI filePath = _fileSystem->get_engine_root_path() + pair.first;
		uicore::TextureInfo textureInfo;
		unsigned char* imageData = stbi_load(filePath.c_str(), &textureInfo.width, &textureInfo.height, NULL, 4);
		if (imageData == NULL)
			LOG_FATAL("Failed to load icon {}", filePath.c_str());
		create_icon(cmdBuffer, cpuBuffers.emplace_back(), sampler, textureInfo, imageData);
		_icons[pair.second] = textureInfo;
		stbi_image_free(imageData);
	}
	
	_rhi->submit(rhi::QueueType::GRAPHICS, true);
	ImGui_ImplVulkan_DestroyFontUploadObjects();

	for (auto& buffer : cpuBuffers)
		_rhi->destroy_buffer(&buffer);
}

void VulkanUIWindowBackend::set_backbuffer(rhi::TextureView* textureView, rhi::Sampler sampler)
{
	if (_descriptorSet == VK_NULL_HANDLE)
	{
		_descriptorSet = ImGui_ImplVulkan_AddTexture(
		   get_vk_obj(&sampler)->get_handle(),
		   get_vk_obj(textureView)->get_handle(),
		   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	else
	{
		ImGui_ImplVulkan_UpdateTexture(
			_descriptorSet,
			get_vk_obj(&sampler)->get_handle(),
			get_vk_obj(textureView)->get_handle(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
}

void VulkanUIWindowBackend::cleanup()
{
	vkDestroyDescriptorPool(_rhi->get_device()->get_device(), _descriptorPool, nullptr);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplWin32_Shutdown();
}

void VulkanUIWindowBackend::draw(rhi::CommandBuffer* cmd)
{
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), get_vk_obj(cmd)->get_handle());
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

void VulkanUIWindowBackend::resize(uint32_t width, uint32_t height)
{
	// TODO
}

void VulkanUIWindowBackend::get_callbacks(rhi::UIWindowBackendCallbacks& callbacks)
{
	callbacks.beginFrameCallback = [this]()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = _mainWindow->get_width();
		io.DisplaySize.y = _mainWindow->get_height();
		ImGui_ImplVulkan_NewFrame();
#ifdef _WIN32
		ImGui_ImplWin32_NewFrame();
#endif
	};

	callbacks.getViewportImageCallback = [this]()->uint64_t
	{
		return (uint64_t)_descriptorSet;
	};

	callbacks.getContextCallback = [this]()->ImGuiContext*
	{
		return ImGui::GetCurrentContext();
	};

	callbacks.getImGuiAllocators = [this]()->rhi::UIWindowBackendCallbacks::ImGuiAllocators
	{
		rhi::UIWindowBackendCallbacks::ImGuiAllocators allocators;
		void* userData;
		ImGui::GetAllocatorFunctions(&allocators.allocFunc, &allocators.freeFunc, &userData);
		return allocators;
	};

	callbacks.getDefaultFont14 = [this]()->ImFont*
	{
		return _fontSize14;
	};

	callbacks.getDefaultFont17 = [this]()->ImFont*
	{
		return _fontSize17;
	};

	callbacks.getContentIcons = [this]()->std::unordered_map<editor::IconType, uicore::TextureInfo>
	{
		return _icons;
	};
}

void VulkanUIWindowBackend::create_icon(rhi::CommandBuffer& cmd, rhi::Buffer& cpuBuffer, rhi::Sampler& sampler, uicore::TextureInfo& iconInfo, unsigned char* imageData)
{
	rhi::Texture texture;
	texture.textureInfo.width = iconInfo.width;
	texture.textureInfo.height = iconInfo.height;
	texture.textureInfo.layersCount = 1;
	texture.textureInfo.mipLevels = 1;
	texture.textureInfo.format = rhi::Format::R8G8B8A8_UNORM;
	texture.textureInfo.memoryUsage = rhi::MemoryUsage::GPU;
	texture.textureInfo.samplesCount = rhi::SampleCount::BIT_1;
	texture.textureInfo.textureDimension = rhi::TextureDimension::TEXTURE2D;
	texture.textureInfo.textureUsage = rhi::ResourceUsage::SAMPLED_TEXTURE | rhi::ResourceUsage::TRANSFER_DST;
	_rhi->create_texture(&texture);
	
	cpuBuffer.bufferInfo.size = iconInfo.width * iconInfo.height * 4;
	cpuBuffer.bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC;
	cpuBuffer.bufferInfo.memoryUsage = rhi::MemoryUsage::CPU;
	_rhi->create_buffer(&cpuBuffer, imageData);

	_rhi->copy_buffer_to_texture(&cmd, &cpuBuffer, &texture);

	rhi::TextureView textureView;
	textureView.viewInfo.baseLayer = 0;
	textureView.viewInfo.baseMipLevel = 0;
	textureView.viewInfo.textureAspect = rhi::TextureAspect::COLOR;
	_rhi->create_texture_view(&textureView, &texture);
	
	iconInfo.textureID64 = (uint64_t)ImGui_ImplVulkan_AddTexture(
		   get_vk_obj(&sampler)->get_handle(),
		   get_vk_obj(&textureView)->get_handle(),
		   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
