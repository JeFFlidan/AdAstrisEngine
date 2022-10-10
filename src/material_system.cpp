#include "material_system.h"
#include "vk_engine.h"

namespace vkutil
{
	VkPipeline PipelineBuilder::build_pipeline(VkDevice device, VkRenderPass pass)
	{
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;

		viewportState.viewportCount = 1;
		viewportState.pViewports = &_viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &_scissor;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.pNext = nullptr;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &_colorBlendAttachment;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = nullptr;

		pipelineInfo.stageCount = _shaderStages.size();
		pipelineInfo.pStages = _shaderStages.data();
		pipelineInfo.pVertexInputState = &_vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &_inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &_rasterizer;
		pipelineInfo.pMultisampleState = &_multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = _pipelineLayout;
		pipelineInfo.renderPass = pass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.pDepthStencilState = &_depthStencil;

		VkPipeline newPipeline;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS)
		{
			return VK_NULL_HANDLE;
		}
		else
		{
			return newPipeline;
		}
	}
}