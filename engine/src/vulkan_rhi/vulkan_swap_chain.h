#pragma once

#include "rhi/resources.h"
#include "vulkan_texture.h"
#include "vulkan_device.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace ad_astris::vulkan
{
	class VK_RHI_API VulkanSwapChain
	{
		public:
			VulkanSwapChain(rhi::SwapChainInfo* swapInfo, VulkanDevice* device);
			void cleanup();

			VkSwapchainKHR get_swap_chain() { return _swapChain; }
			VkFormat get_format() { return _format; }
			std::vector<rhi::TextureView>& get_texture_views() { return _textureViews; }
			uint32_t get_buffers_count() { return _textureViews.size(); }
		
		private:
			VulkanDevice* _device{ nullptr };
			VkSwapchainKHR _swapChain;
			std::vector<rhi::TextureView> _textureViews;
			std::vector<std::unique_ptr<rhi::Texture>> _textures;
			std::vector<std::unique_ptr<VulkanTextureView>> _vulkanTextureViews;

			// Only for test
			VkFormat _format;
	};
}
