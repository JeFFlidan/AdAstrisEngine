#include "vulkan_ui_window_backend.h"
#include "vulkan_common.h"
#include "vulkan_queue.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_win32.h"

using namespace ad_astris;
using namespace vulkan;

void VulkanUIWindowBackend::init(rhi::UIWindowBackendInitContext& initContext)
{
	_rhi = static_cast<VulkanRHI*>(initContext.rhi);
	_mainWindow = initContext.window;
	
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
	ImGui_ImplWin32_InitForVulkan(_mainWindow->get_hWnd());
#endif

	ImGui_ImplVulkan_InitInfo vulkanInitInfo{};
	vulkanInitInfo.Instance = _rhi->get_instance();
	vulkanInitInfo.PhysicalDevice = _rhi->get_device()->get_physical_device();
	vulkanInitInfo.Device = _rhi->get_device()->get_device();
	vulkanInitInfo.Queue = _rhi->get_device()->get_graphics_queue()->get_queue();
	vulkanInitInfo.DescriptorPool = _descriptorPool;
	vulkanInitInfo.MinImageCount = 3;
	vulkanInitInfo.ImageCount = 3;
	vulkanInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	vulkanInitInfo.UseDynamicRendering = true;
	vulkanInitInfo.ColorAttachmentFormat = _rhi->get_swap_chain()->get_format();
	
	ImGui_ImplVulkan_Init(&vulkanInitInfo, VK_NULL_HANDLE);
	rhi::CommandBuffer cmdBuffer;
	_rhi->begin_command_buffer(&cmdBuffer, rhi::QueueType::GRAPHICS);
	ImGui_ImplVulkan_CreateFontsTexture(get_vk_obj(&cmdBuffer)->get_handle());
	_rhi->submit(rhi::QueueType::GRAPHICS);
	_rhi->wait_fences();
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void VulkanUIWindowBackend::set_backbuffer(rhi::TextureView* textureView, rhi::Sampler* sampler)
{
	if (_descriptorSet == VK_NULL_HANDLE)
	{
		_descriptorSet = ImGui_ImplVulkan_AddTexture(
		   get_vk_obj(sampler)->get_handle(),
		   get_vk_obj(textureView)->get_handle(),
		   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	else
	{
		ImGui_ImplVulkan_UpdateTexture(
			_descriptorSet,
			get_vk_obj(sampler)->get_handle(),
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
}
