#pragma once

#include "rhi/resources.h"
#include "vulkan_texture.h"
#include "vulkan_device.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace ad_astris::vulkan
{
	class VulkanSwapChain
	{
		public:
			VulkanSwapChain(rhi::SwapChainInfo* swapChainInfo, VulkanDevice* device);
		
			void recreate(rhi::SwapChainInfo* swapChainInfo);
			void recreate(uint32_t width, uint32_t height);

			VkSwapchainKHR get_swap_chain() { return _swapChain.swapchain; }
			VkFormat get_format() { return _swapChain.image_format; }
			uint32_t get_buffers_count() { return _swapChain.get_images().value().size(); }
			uint32_t get_width() { return _swapChainInfo.width; }
			uint32_t get_height() { return _swapChainInfo.height; }

			VkImage get_image_handle(uint32_t currentFrameIndex)
			{
				if (_images.empty())
					_images = _swapChain.get_images().value();
				return _images[currentFrameIndex];
			}

			VkImageView get_image_view_handle(uint32_t currentFrameIndex)
			{
				if (_imageViews.empty())
					_imageViews = _swapChain.get_image_views().value();
				return _imageViews[currentFrameIndex];
			}
		
		private:
			VulkanDevice* _device{ nullptr };
			vkb::Swapchain _swapChain;
			rhi::SwapChainInfo _swapChainInfo;
			std::vector<VkImageView> _imageViews;
			std::vector<VkImage> _images;
		
			void create_swap_chain_internal(VkSwapchainKHR oldSwapChain = VK_NULL_HANDLE);
	};
}
