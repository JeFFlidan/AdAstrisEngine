#include "vulkan_swap_chain.h"
#include "profiler/logger.h"
#include <VkBootstrap.h>

#include "fmt/color.h"

using namespace ad_astris;

vulkan::VulkanSwapChain::VulkanSwapChain(rhi::SwapChainInfo* swapInfo, VulkanDevice* device) : _device(device)
{
	if (swapInfo->buffersCount < 2 || swapInfo->buffersCount > 3)
	{
		LOG_ERROR("VulkanSwapChain::VulkanSwapChain(): Invalid amount of buffers for swap chain")
		return;
	}
	
	VkPresentModeKHR presentMode;
	
	if (swapInfo->buffersCount == 3)
	{
		presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	}
	else if (swapInfo->sync)
	{
		presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
	}
	else if (!swapInfo->sync)
	{
		presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}
	
	VkSurfaceFormatKHR format;
	format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	format.format = VK_FORMAT_B8G8R8A8_SRGB;
	
	vkb::SwapchainBuilder swapchainBuilder{
		_device->get_physical_device(),
		_device->get_device(),
		_device->get_surface() };
	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.set_desired_present_mode(presentMode)
		.set_desired_extent(swapInfo->width, swapInfo->height)
		.set_desired_format(format)
		.build()
		.value();

	_swapChain = vkbSwapchain.swapchain;
	_images = vkbSwapchain.get_images().value();
	_imageViews = vkbSwapchain.get_image_views().value();
	_format = vkbSwapchain.image_format;

	for (int i = 0; i != swapInfo->buffersCount; ++i)
	{
		rhi::TextureInfo texInfo;
		texInfo.samplesCount = rhi::SampleCount::BIT_1;
		texInfo.width = swapInfo->width;
		texInfo.height = swapInfo->height;
		texInfo.format = rhi::Format::B8G8R8A8_SRGB;
		rhi::Texture* text = new rhi::Texture();
		text->textureInfo = texInfo;
		VkImageView* view = new VkImageView();
		*view = _imageViews[i];
		rhi::TextureView texView;
		texView.texture = text;
		texView.handle = view;
		_textureViews.push_back(texView);
	}
}

void vulkan::VulkanSwapChain::cleanup()
{
	for (auto& view : _textureViews)
	{
		VkImageView* imageView = static_cast<VkImageView*>(view.handle);
		vkDestroyImageView(_device->get_device(), *imageView, nullptr);
		delete imageView;
		delete view.texture;
	}
	vkDestroySwapchainKHR(_device->get_device(), _swapChain, nullptr);
}
