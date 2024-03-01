#pragma once

#include "core/platform.h"
#include "vulkan_device.h"
#include "vulkan_texture.h"
#include "vulkan_surface.h"
#include "vulkan_object.h"
#include "rhi/resources.h"
#include <vector>

namespace ad_astris::vulkan
{
	class VulkanCommandBuffer;
	
	class VulkanSwapChain : public IVulkanObject
	{
		public:
			VulkanSwapChain() = default;
			VulkanSwapChain(VulkanDevice* device, rhi::SwapChainInfo* swapChainInfo, acore::IWindow* window);

			void prepare_for_drawing(VulkanCommandBuffer* cmd);
			void prepare_for_presenting(VulkanCommandBuffer* cmd);
			void recreate(rhi::SwapChainInfo* swapChainInfo);
			void recreate();
			void destroy(VulkanDevice* device) override;

			VkSwapchainKHR get_handle() const { return _swapChain; }
			VkSemaphore get_acquire_semaphore() const { return _acquireSemaphores[_frameIndex]; }
			uint32_t get_buffers_count() const { return _images.size(); }
			uint32_t get_width() const { return _swapChainInfo.width; }
			uint32_t get_height() const { return _swapChainInfo.height; }
			VkFormat get_format() const { return _format; }
			uint32_t get_frame_index() const { return _frameIndex; }

			VkImage get_image_handle() const
			{
				return _images[_frameIndex];
			}

			VkImageView get_image_view_handle() const
			{
				return _imageViews[_frameIndex];
			}
		
		private:
			VulkanDevice* _device{ nullptr };
			std::unique_ptr<VulkanSurface> _surface{ nullptr };
			VkSwapchainKHR _swapChain{ VK_NULL_HANDLE };
			std::vector<VkSemaphore> _acquireSemaphores{ VK_NULL_HANDLE };
			VkFormat _format;
			rhi::SwapChainInfo _swapChainInfo;
			std::vector<VkImageView> _imageViews;
			std::vector<VkImage> _images;
			uint32_t _frameIndex{ 0 };
			acore::IWindow* _window{ nullptr };
		
			void create_swap_chain_internal();
			void execute_image_barrier(VulkanCommandBuffer* cmd, VkImageMemoryBarrier2& imageBarrier);
	};
}
