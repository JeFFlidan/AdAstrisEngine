#include "vulkan_swap_chain.h"
#include "vulkan_common.h"
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
		presentMode = VK_PRESENT_MODE_FIFO_KHR;
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
	auto images = vkbSwapchain.get_images().value();
	auto imageViews = vkbSwapchain.get_image_views().value();
	swapInfo->buffersCount = imageViews.size();
	_format = vkbSwapchain.image_format;

	for (int i = 0; i != imageViews.size(); ++i)
	{
		_textures.emplace_back(new rhi::Texture());
		_textures.back()->textureInfo.samplesCount = rhi::SampleCount::BIT_1;
		_textures.back()->textureInfo.width = swapInfo->width;
		_textures.back()->textureInfo.height = swapInfo->height;
		_textures.back()->textureInfo.format = rhi::Format::B8G8R8A8_SRGB;
		_vulkanTextureViews.emplace_back(new VulkanTextureView());
		_vulkanTextureViews.back()->set_handle(imageViews[i]);
		rhi::TextureView texView;
		texView.texture = _textures.back().get();
		texView.handle = _vulkanTextureViews.back().get();
		_textureViews.push_back(texView);
	}
}

void vulkan::VulkanSwapChain::cleanup()
{
	for (auto& view : _textureViews)
	{
		VulkanTextureView* vulkanTextureView = get_vk_obj(&view);
		vkDestroyImageView(_device->get_device(), vulkanTextureView->get_handle(), nullptr);
	}
	vkDestroySwapchainKHR(_device->get_device(), _swapChain, nullptr);
}
