#pragma once

#include "vk_initializers.h"
#include "material_system/shader.h"
#include "vk_mesh.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace engine
{
	class GraphicsPipelineBuilder
	{
		public:
			GraphicsPipelineBuilder() = default;
			GraphicsPipelineBuilder(VkDevice device);

			VkPipeline build(VkRenderPass renderPass, VkPipelineLayout layout);

			// Get info to tweak some extra settings. Maybe I'll remove it
			VkGraphicsPipelineCreateInfo& get_info() { return _pipelineInfo; }

			GraphicsPipelineBuilder& setup_shader_stages(std::vector<ShaderEffect::ShaderStage> stages);
			GraphicsPipelineBuilder& setup_vertex_input_state(VertexInputDescription& vertexDescription);
			GraphicsPipelineBuilder& setup_assembly_state(VkPrimitiveTopology topology);
			GraphicsPipelineBuilder& setup_tessel_state(); // I will implement it in the future
			GraphicsPipelineBuilder& setup_dynamic_viewport_state();
			GraphicsPipelineBuilder& setup_static_viewport_state(
				uint32_t viewportCount,
				VkViewport* viewports,
				uint32_t scissorCount,
				VkRect2D* scissors);
			GraphicsPipelineBuilder& setup_rasterization_state(
				VkPolygonMode polygonMode,
				VkCullModeFlags cullMode,
				VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
				VkBool32 isBiasEnabled = VK_FALSE
			); // TODO
			GraphicsPipelineBuilder& setup_multisample_state();
			GraphicsPipelineBuilder& setup_depth_state(
				VkBool32 depthEnable,
				VkBool32 depthWrite,
				VkCompareOp compare);
			GraphicsPipelineBuilder& setup_color_blend_state_default(uint32_t count = 1);
			// For pipelines without blending
			GraphicsPipelineBuilder& setup_color_blend_state(VkColorComponentFlags color, uint32_t count = 1);
			// For pipelines with blending.
			GraphicsPipelineBuilder& setup_dynamic_state(bool dynamicViewport, bool depthBias);

		private:
			VkDevice _device = VK_NULL_HANDLE;
			VkGraphicsPipelineCreateInfo _pipelineInfo;

			// graphics pipeline
			std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
			VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
			VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
			VkViewport _viewport;
			VkRect2D _scissor;
			VkPipelineViewportStateCreateInfo _viewportState;
			VkPipelineDepthStencilStateCreateInfo _depthStencil;
			VkPipelineRasterizationStateCreateInfo _rasterizer;
			VkPipelineMultisampleStateCreateInfo _multisampling;
			VkPipelineColorBlendStateCreateInfo _blendState;
			std::vector<VkPipelineColorBlendAttachmentState> _colorBlendManyAttachments;
			std::vector<VkDynamicState> _dynamicStates;
			VkPipelineDynamicStateCreateInfo _dynamicState;
			VertexInputDescription _inputDescription;
	};

	class ComputePipelineBuilder
	{
		public:
			ComputePipelineBuilder(VkDevice device);
			VkPipeline build(ShaderEffect::ShaderStage stage, VkPipelineLayout layout);
			VkComputePipelineCreateInfo& get_info() { return _pipelineInfo; }

		private:
			VkDevice _device;
			VkComputePipelineCreateInfo _pipelineInfo;
	};

	class RayTracingPipelineBuilder
	{
		public:
			RayTracingPipelineBuilder(VkDevice device);
			//TODO
		private:
			VkDevice _device;
	};
}
