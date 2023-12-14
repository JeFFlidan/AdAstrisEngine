#pragma once

#include "vulkan_object.h"
#include "vulkan_device.h"
#include "rhi/resources.h"
#include <vulkan/vulkan.h>

namespace ad_astris::vulkan
{
	class VulkanRenderPass : public IVulkanObject
	{
		public:
			VulkanRenderPass() = default;
			VulkanRenderPass(VulkanDevice* device, rhi::RenderPassInfo* passInfo);

			void destroy(VulkanDevice* device) override;

			VkRenderPassBeginInfo get_begin_info(rhi::ClearValues& rhiClearValue, uint32_t imageIndex = 0);

			VkRenderPass get_handle()
			{
				return _renderPass;
			}
		
		private:
			VulkanDevice* _device;
			VkRenderPass _renderPass;
			std::vector<VkFramebuffer> _framebuffers;
			VkExtent2D _extent;
			uint32_t _colorAttachCount{ 0 };
			uint32_t _depthAttachCount{ 0 };

			void create_render_pass(rhi::RenderPassInfo* passInfo);
			void create_framebuffer(std::vector<rhi::RenderBuffer>& renderBuffers);
	};
}