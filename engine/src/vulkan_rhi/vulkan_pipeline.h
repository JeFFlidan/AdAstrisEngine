#pragma once

#include "rhi/resources.h"
#include "vulkan_device.h"
#include <vulkan/vulkan.h>

namespace ad_astris::vulkan
{
	class VK_RHI_API VulkanPipeline : public IVulkanPipeline
	{
		public:
			VulkanPipeline(VulkanDevice* device, rhi::GraphicsPipelineInfo* info);
			VulkanPipeline(VulkanDevice* device, rhi::ComputePipelineInfo* info);
			~VulkanPipeline();

			VkPipeline get_handle() { return _pipeline; }
			VkPipelineLayout get_layout() { return _layout; }
			rhi::PipelineType get_type() { return _type; }
		
		private:
			VulkanDevice* _device;
			VkPipeline _pipeline;
			VkPipelineLayout _layout;
			rhi::PipelineType _type{ rhi::PipelineType::UNDEFINED };

			void create_graphics_pipeline(rhi::GraphicsPipelineInfo* info);
			void create_compute_pipeline(rhi::ComputePipelineInfo* info);
	};
}