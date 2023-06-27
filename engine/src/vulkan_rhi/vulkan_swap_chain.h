#pragma once

#include "rhi/resources.h"
#include "vulkan_device.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace ad_astris::vulkan
{
	class VK_RHI_API VulkanSwapChain : public IVulkanSwapChain
	{
		public:
			VulkanSwapChain(rhi::SwapChainInfo* swapInfo, VulkanDevice* device);
			~VulkanSwapChain();

			VkSwapchainKHR get_swap_chain() { return _swapChain; }
			VkFormat get_format() { return _format; }
			std::vector<rhi::TextureView>& get_texture_views() { return _textureViews; }

			// Only for test
			std::vector<VkImageView>& get_image_views() { return _imageViews; }
			std::vector<VkImage> get_images() { return _images; }
		private:
			VkSwapchainKHR _swapChain;
			std::vector<rhi::TextureView> _textureViews; 
			VulkanDevice* _device{ nullptr };

			// Only for test
			VkFormat _format;
			std::vector<VkImageView> _imageViews;
			std::vector<VkImage> _images;
	};
}
