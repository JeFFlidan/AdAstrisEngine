#include "vk_pipeline.h"

#include "profiler/logger.h"

#include <cassert>

using namespace ad_astris;

GraphicsPipelineBuilder::GraphicsPipelineBuilder(VkDevice device) : _device(device)
{
	_pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	_pipelineInfo.flags = 0;
	_pipelineInfo.pNext = nullptr;
}

VkPipeline GraphicsPipelineBuilder::build(VkRenderPass renderPass, VkPipelineLayout layout)
{
	_vertexInputInfo.vertexBindingDescriptionCount = _inputDescription.bindings.size();
	_vertexInputInfo.pVertexBindingDescriptions = _inputDescription.bindings.data();
	_vertexInputInfo.vertexAttributeDescriptionCount = _inputDescription.attributes.size();
	_vertexInputInfo.pVertexAttributeDescriptions = _inputDescription.attributes.data();

	_dynamicState = vkinit::dynamic_state_create_info(_dynamicStates.data(), _dynamicStates.size());

	_blendState.attachmentCount = _colorBlendManyAttachments.size();
	_blendState.pAttachments = _colorBlendManyAttachments.data();
	
	if (_device == VK_NULL_HANDLE)
	{
		LOG_ERROR("Null handle")
		return VK_NULL_HANDLE;
	}
	_pipelineInfo.renderPass = renderPass;
	_pipelineInfo.subpass = 0;
	_pipelineInfo.layout = layout;

	_pipelineInfo.pStages = _shaderStages.data();
	_pipelineInfo.stageCount = _shaderStages.size();
	_pipelineInfo.pVertexInputState = &_vertexInputInfo;
	_pipelineInfo.pInputAssemblyState = &_inputAssembly;
	_pipelineInfo.pViewportState = &_viewportState;
	_pipelineInfo.pRasterizationState = &_rasterizer;
	_pipelineInfo.pMultisampleState = &_multisampling;
	_pipelineInfo.pColorBlendState = &_blendState;
	_pipelineInfo.pDepthStencilState = &_depthStencil;
	_pipelineInfo.pDynamicState = &_dynamicState;
	_pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline pipeline;
	if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &_pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		LOG_FATAL("Failed to create graphics pipeline")
	}
	_shaderStages.clear();

	return pipeline;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setup_shader_stages(std::vector<ShaderEffect::ShaderStage> stages)
{
	_shaderStages.clear();
	for (auto& stage : stages)
	{
		_shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(stage.stage, stage.shader->get_shader_module()));
	}

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setup_vertex_input_state(VertexInputDescription& vertexDescription)
{
	_inputDescription = vertexDescription;
	_vertexInputInfo = vkinit::vertex_input_state_create_info();

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setup_assembly_state(VkPrimitiveTopology topology)
{
	_inputAssembly = vkinit::input_assembly_create_info(topology);
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setup_tessel_state()
{
	// TODO
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setup_dynamic_viewport_state()
{
	_viewportState = {};
	_viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	_viewportState.pScissors = nullptr;
	_viewportState.scissorCount = 1;
	_viewportState.pViewports = nullptr;
	_viewportState.viewportCount = 1;

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setup_static_viewport_state(
	uint32_t viewportCount,
	VkViewport* viewports,
	uint32_t scissorCount,
	VkRect2D* scissors)
{
	_viewportState = {};
	_viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	_viewportState.viewportCount = viewportCount;
	_viewportState.pViewports = viewports;
	_viewportState.pScissors = scissors;
	_viewportState.scissorCount = scissorCount;

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setup_rasterization_state(
	VkPolygonMode polygonMode,
	VkCullModeFlags cullMode,
	VkFrontFace frontFace,
	VkBool32 isBiasEnabled
)
{
	_rasterizer = vkinit::rasterization_state_create_info(polygonMode, cullMode, frontFace);
	_rasterizer.depthBiasEnable = isBiasEnabled;

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setup_multisample_state()
{
	// I think I should remake this part in the future
	_multisampling = vkinit::multisampling_state_create_info();

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setup_depth_state(
	VkBool32 depthEnable,
	VkBool32 depthWrite,
	VkCompareOp compare)
{
	_depthStencil = vkinit::depth_stencil_create_info(depthEnable, depthWrite, compare);

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setup_color_blend_state_default(uint32_t count)
{
	_colorBlendManyAttachments.clear();
	_blendState = {};
	_blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	_blendState.pNext = nullptr;
	_blendState.logicOpEnable = VK_FALSE;
	_blendState.logicOp = VK_LOGIC_OP_COPY;

	for (int i = 0; i != count; ++i)
	{
		_colorBlendManyAttachments.push_back(vkinit::color_blend_attachment_state());
		_colorBlendManyAttachments[i].blendEnable = VK_FALSE;
	}
	
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setup_color_blend_state(VkColorComponentFlags color, uint32_t count)
{
	_colorBlendManyAttachments.clear();
	// TODO
	_blendState = vkinit::color_blend_state_create_info();
	_blendState.logicOpEnable = VK_LOGIC_OP_COPY;
	_blendState.logicOpEnable = VK_FALSE;

	for (int i = 0; i != count; ++i)
	{
		_colorBlendManyAttachments.push_back(vkinit::color_blend_attachment_state());
		_colorBlendManyAttachments[0].colorWriteMask = color;
	}

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setup_dynamic_state(bool dynamicViewport, bool depthBias)
{
	_dynamicStates.clear();
	if (dynamicViewport)
	{
		_dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		_dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	}
	if (depthBias)
	{
		_dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	}

	return *this;
}

ComputePipelineBuilder::ComputePipelineBuilder(VkDevice device) : _device(device)
{
	_pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	_pipelineInfo.pNext = nullptr;
	_pipelineInfo.flags = 0;
}

VkPipeline ComputePipelineBuilder::build(ShaderEffect::ShaderStage stage, VkPipelineLayout layout)
{
	_pipelineInfo.layout = layout;
	if (stage.stage != VK_SHADER_STAGE_COMPUTE_BIT)
	{
		LOG_FATAL("Shader stage isn't VK_SHADER_STAGE_COMPUTE_BIT for compute pipeline");
	}

	_pipelineInfo.stage = vkinit::pipeline_shader_stage_create_info(
		stage.stage,
		stage.shader->get_shader_module());

	VkPipeline pipeline;
	if (vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1, &_pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		LOG_FATAL("Failed to create compute pipeline");
	}

	return pipeline;
}
