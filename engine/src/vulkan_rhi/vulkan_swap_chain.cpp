#include "vulkan_swap_chain.h"
#include "vulkan_common.h"
#include "profiler/logger.h"
#include <vkbootstrap/VkBootstrap.h>

using namespace ad_astris;

vulkan::VulkanSwapChain::VulkanSwapChain(rhi::SwapChainInfo* swapChainInfo, VulkanDevice* device) : _device(device)
{
	assert(swapChainInfo != nullptr);
	_swapChainInfo = *swapChainInfo;
	create_swap_chain_internal();
	_imageViews = _swapChain.get_image_views().value();
	_images = _swapChain.get_images().value();
}

void vulkan::VulkanSwapChain::recreate(rhi::SwapChainInfo* swapChainInfo)
{
	_swapChainInfo = *swapChainInfo;
	vkb::Swapchain oldSwapChain = _swapChain;
	create_swap_chain_internal(_swapChain.swapchain);
	create_swap_chain_internal(_swapChain.swapchain);
	
	for (auto& imageView : _imageViews)
		vkDestroyImageView(_device->get_device(), imageView, nullptr);
	_imageViews.clear();
	_images.clear();
	vkb::destroy_swapchain(oldSwapChain);
}

void vulkan::VulkanSwapChain::recreate(uint32_t width, uint32_t height)
{
	_swapChainInfo.width = width;
	_swapChainInfo.height = height;
	vkb::Swapchain oldSwapChain = _swapChain;
	create_swap_chain_internal(_swapChain.swapchain);
	
	for (auto& imageView : _imageViews)
		vkDestroyImageView(_device->get_device(), imageView, nullptr);
	_imageViews.clear();
	_images.clear();
	vkb::destroy_swapchain(oldSwapChain);
}

void vulkan::VulkanSwapChain::create_swap_chain_internal(VkSwapchainKHR oldSwapChain)
{
	if (_swapChainInfo.buffersCount < 2 || _swapChainInfo.buffersCount > 3)
	{
		LOG_ERROR("VulkanSwapChain::VulkanSwapChain(): Invalid amount of buffers for swap chain")
		return;
	}
	
	VkPresentModeKHR presentMode;
	
	if (_swapChainInfo.buffersCount == 3)
	{
		presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	}
	else if (_swapChainInfo.sync)
	{
		presentMode = VK_PRESENT_MODE_FIFO_KHR;
	}
	else if (!_swapChainInfo.sync)
	{
		presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}
	
	VkSurfaceFormatKHR format;
	format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	format.format = VK_FORMAT_B8G8R8A8_UNORM;
	
	vkb::SwapchainBuilder swapchainBuilder{
		_device->get_physical_device(),
		_device->get_device(),
		_device->get_surface() };
	swapchainBuilder.set_desired_present_mode(presentMode);
	swapchainBuilder.set_desired_extent(_swapChainInfo.width, _swapChainInfo.height);
	swapchainBuilder.set_desired_format(format);
	if (oldSwapChain != VK_NULL_HANDLE)
		swapchainBuilder.set_old_swapchain(oldSwapChain);
	_swapChain = swapchainBuilder.build().value();
}
