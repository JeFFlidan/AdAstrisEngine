#include "vulkan_swap_chain.h"
#include "vulkan_common.h"
#include "profiler/logger.h"

using namespace ad_astris;
using namespace vulkan;

VulkanSwapChain::VulkanSwapChain(
	VulkanDevice* device,
	rhi::SwapChainInfo* swapChainInfo,
	acore::IWindow* window)
		: _device(device), _window(window)
{
	_surface = std::make_unique<VulkanSurface>(device, _window);
	_swapChainInfo = *swapChainInfo;
	create_swap_chain_internal();
	*swapChainInfo = _swapChainInfo;
	create_semaphore(_device->get_device(), &_acquireSemaphore);
}

void VulkanSwapChain::recreate(rhi::SwapChainInfo* swapChainInfo)
{
	_swapChainInfo = *swapChainInfo;
	create_swap_chain_internal();
	*swapChainInfo = _swapChainInfo;
}

void VulkanSwapChain::prepare_for_drawing(VulkanCommandBuffer* cmd)
{
	VkResult res = vkAcquireNextImageKHR(_device->get_device(), _swapChain, 1000000000, _acquireSemaphore, VK_NULL_HANDLE, &_frameIndex);
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
		recreate();

	VkImageMemoryBarrier2 imageBarrier{};
	imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageBarrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	imageBarrier.srcAccessMask = VK_ACCESS_2_NONE;
	imageBarrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
	execute_image_barrier(cmd, imageBarrier);
}

void VulkanSwapChain::prepare_for_presenting(VulkanCommandBuffer* cmd)
{
	VkImageMemoryBarrier2 imageBarrier{};
	imageBarrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	imageBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	imageBarrier.dstAccessMask = VK_ACCESS_2_NONE;
	execute_image_barrier(cmd, imageBarrier);
}

void VulkanSwapChain::recreate()
{
	_swapChainInfo.width = _window->get_width();
	_swapChainInfo.height = _window->get_height();
	create_swap_chain_internal();
}

void VulkanSwapChain::destroy(VulkanDevice* device)
{
	if (_acquireSemaphore != VK_NULL_HANDLE)
		vkDestroySemaphore(device->get_device(), _acquireSemaphore, nullptr);
	for (auto& view : _imageViews)
		vkDestroyImageView(device->get_device(), view, nullptr);
	if (_swapChain != VK_NULL_HANDLE)
		vkDestroySwapchainKHR(device->get_device(), _swapChain, nullptr);
	if (_surface != nullptr)
		_surface->cleanup(device->get_instance());
}

void VulkanSwapChain::create_swap_chain_internal()
{
	const VkSurfaceCapabilitiesKHR surfaceCapabilities = _surface->get_capabilities(_device);
	const std::vector<VkSurfaceFormat2KHR>& availableFormats = _surface->get_available_formats();
	const std::vector<VkPresentModeKHR>& availablePresentModes = _surface->get_available_present_modes();

	VkSurfaceFormat2KHR swapChainFormat{};
	swapChainFormat.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
	swapChainFormat.surfaceFormat.format = ::get_format(_swapChainInfo.format);
	swapChainFormat.surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	bool isSwapChainFormatValid = false;
	
	for (const auto& availableFormat : availableFormats)
	{
		if (!_swapChainInfo.useHDR && availableFormat.surfaceFormat.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			continue;
		if (swapChainFormat.surfaceFormat.format == availableFormat.surfaceFormat.format)
		{
			swapChainFormat = availableFormat;
			isSwapChainFormatValid = true;
			break;
		}
	}
	if (!isSwapChainFormatValid)
	{
		_swapChainInfo.format = rhi::Format::B8G8R8A8_UNORM;
		_swapChainInfo.colorSpace = rhi::ColorSpace::SRGB;
		swapChainFormat.surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		swapChainFormat.surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	}

	switch (swapChainFormat.surfaceFormat.colorSpace)
	{
		case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
			_swapChainInfo.colorSpace = rhi::ColorSpace::SRGB;
			break;
		case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
			_swapChainInfo.colorSpace = rhi::ColorSpace::HDR_LINEAR;
			break;
		case VK_COLOR_SPACE_HDR10_ST2084_EXT:
			_swapChainInfo.colorSpace = rhi::ColorSpace::HDR10_ST2084;
			break;
	}

	_format = swapChainFormat.surfaceFormat.format;

	// TODO test swap chain recreation with old swap chain changing color space
	
	uint32_t imageCount = std::max(_swapChainInfo.buffersCount, surfaceCapabilities.minImageCount);
	if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
	{
		imageCount = surfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapChainCreateInfo{};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = _surface->get_handle();
	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageFormat = swapChainFormat.surfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = swapChainFormat.surfaceFormat.colorSpace;
	swapChainCreateInfo.imageExtent = { _swapChainInfo.width, _swapChainInfo.height };
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	
	swapChainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	if (!_swapChainInfo.vSync)
	{
		for (auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				swapChainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				swapChainCreateInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}

	swapChainCreateInfo.clipped = VK_TRUE;
	swapChainCreateInfo.oldSwapchain = _swapChain;

	VK_CHECK(vkCreateSwapchainKHR(_device->get_device(), &swapChainCreateInfo, nullptr, &_swapChain));

	if (swapChainCreateInfo.oldSwapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(_device->get_device(), swapChainCreateInfo.oldSwapchain, nullptr);
	}
	
	VK_CHECK(vkGetSwapchainImagesKHR(_device->get_device(), _swapChain, &_swapChainInfo.buffersCount, nullptr));
	_images.resize(_swapChainInfo.buffersCount);
	VK_CHECK(vkGetSwapchainImagesKHR(_device->get_device(), _swapChain, &_swapChainInfo.buffersCount, _images.data()));

	for (auto& view : _imageViews)
	{
		vkDestroyImageView(_device->get_device(), view, nullptr);
	}

	_imageViews.resize(_swapChainInfo.buffersCount);
	
	for (size_t i = 0; i != _swapChainInfo.buffersCount; ++i)
	{
		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = _images[i];
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = _format;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(_device->get_device(), &viewCreateInfo, nullptr, &_imageViews[i]));
	}
}

void VulkanSwapChain::execute_image_barrier(VulkanCommandBuffer* cmd, VkImageMemoryBarrier2& imageBarrier)
{
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	imageBarrier.image = _images[_frameIndex];
	imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
	imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
	imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	imageBarrier.subresourceRange.baseArrayLayer = 0;
	imageBarrier.subresourceRange.baseMipLevel = 0;
	imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	VkDependencyInfo dependencyInfo{};
	dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependencyInfo.memoryBarrierCount = 0;
	dependencyInfo.pMemoryBarriers = nullptr;
	dependencyInfo.pBufferMemoryBarriers = nullptr;
	dependencyInfo.bufferMemoryBarrierCount = 0;
	dependencyInfo.imageMemoryBarrierCount = 1;
	dependencyInfo.pImageMemoryBarriers = &imageBarrier;
	vkCmdPipelineBarrier2(cmd->get_handle(), &dependencyInfo);
}
