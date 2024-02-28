#pragma once

#include "vulkan_device.h"
#include "vulkan_texture.h"
#include "vulkan_surface.h"
#include "rhi/resources.h"
#include <vector>

namespace ad_astris::vulkan
{
	class VulkanSwapChain
	{
		public:
			VulkanSwapChain(VulkanDevice* device, VulkanSurface* surface, rhi::SwapChainInfo* swapChainInfo);
		
			void recreate(rhi::SwapChainInfo* swapChainInfo);
			void recreate(uint32_t width, uint32_t height);

			VkSwapchainKHR get_handle() const { return _swapChain; }
			// VkFormat get_format() { return _swapChain.image_format; }
			uint32_t get_buffers_count() const { return _images.size(); }
			uint32_t get_width() const { return _swapChainInfo.width; }
			uint32_t get_height() const { return _swapChainInfo.height; }
			VkFormat get_format() const { return _format; }

			VkImage get_image_handle(uint32_t currentFrameIndex)
			{
				return _images[currentFrameIndex];
			}

			VkImageView get_image_view_handle(uint32_t currentFrameIndex)
			{
				return _imageViews[currentFrameIndex];
			}
		
		private:
			VulkanDevice* _device{ nullptr };
			VulkanSurface* _surface{ nullptr };
			VkSwapchainKHR _swapChain{ VK_NULL_HANDLE };
			VkFormat _format;
			rhi::SwapChainInfo _swapChainInfo;
			std::vector<VkImageView> _imageViews;
			std::vector<VkImage> _images;
		
			void create_swap_chain_internal();
	};
}
