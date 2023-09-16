#pragma once

#include "rhi/resources.h"
#include "vulkan_device.h"
#include <vulkan/vulkan.h>

namespace ad_astris::vulkan
{
	class VulkanPipeline
	{
		public:
			VulkanPipeline(VulkanDevice* device, rhi::GraphicsPipelineInfo* info, VkPipelineCache pipelineCache);
			VulkanPipeline(VulkanDevice* device, rhi::ComputePipelineInfo* info, VkPipelineCache pipelineCache);

			void cleanup();

			VkPipeline get_handle() { return _pipeline; }
			VkPipelineLayout get_layout() { return _layout; }
			rhi::PipelineType get_type() { return _type; }
		
		private:
			VulkanDevice* _device;
			VkPipeline _pipeline;
			VkPipelineLayout _layout;
			rhi::PipelineType _type{ rhi::PipelineType::UNDEFINED };

			void create_graphics_pipeline(rhi::GraphicsPipelineInfo* info, VkPipelineCache pipelineCache);
			void create_compute_pipeline(rhi::ComputePipelineInfo* info, VkPipelineCache pipelineCache);
	};
}