#pragma once

#include "rhi/resources.h"
#include "vulkan_device.h"
#include "vulkan_pipeline_layout_cache.h"
#include <vulkan/vulkan.h>

namespace ad_astris::vulkan
{
	class VulkanPipeline
	{
		public:
			VulkanPipeline(
				VulkanDevice* device,
				rhi::GraphicsPipelineInfo* info,
				VkPipelineCache pipelineCache,
				VulkanPipelineLayoutCache* layoutCache);
			VulkanPipeline(
				VulkanDevice* device,
				rhi::ComputePipelineInfo* info,
				VkPipelineCache pipelineCache,
				VulkanPipelineLayoutCache* layoutCache);

			void cleanup();
			void bind(VkCommandBuffer cmd, uint32_t frameIndex);

			VkPipeline get_handle() { return _pipeline; }
			rhi::PipelineType get_type() { return _type; }
		
		private:
			VulkanDevice* _device;
			VkPipeline _pipeline;
			VulkanPipelineLayout* _layout{ nullptr };
			rhi::PipelineType _type{ rhi::PipelineType::UNDEFINED };

			void create_graphics_pipeline(
				rhi::GraphicsPipelineInfo* info,
				VkPipelineCache pipelineCache,
				VulkanPipelineLayoutCache* layoutCache);
			void create_compute_pipeline(
				rhi::ComputePipelineInfo* info,
				VkPipelineCache pipelineCache,
				VulkanPipelineLayoutCache* layoutCache);
	};
}