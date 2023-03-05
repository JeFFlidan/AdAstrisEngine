#include "vulkan_rhi.h"
#include "vulkan_common.h"
#include "vulkan_buffer.h"
#include "vulkan_texture.h"
#include "vulkan_shader.h"
#include "vulkan_renderer/vk_initializers.h"

#include "profiler/logger.h"
#include <VkBootstrap.h>

#include "imstb_rectpack.h"
#include "vulkan_texture.h"

using namespace ad_astris;

void vulkan::VulkanRHI::init(void* window)
{
	vkb::Instance vkbInstance = create_instance();
	_instance = vkbInstance.instance;
	_debugMessenger = vkbInstance.debug_messenger;
	_vulkanDevice.init(vkbInstance, window);
	create_allocator();
}

void vulkan::VulkanRHI::cleanup()
{
	
}

void vulkan::VulkanRHI::create_buffer(rhi::Buffer* buffer, rhi::BufferInfo* bufInfo, uint64_t size, void* data)
{
	if (!buffer)
	{
		LOG_ERROR("Can't create buffer if buffer parameter is invalid")
		return;
	}
	
	VkBufferUsageFlags bufferUsage = get_buffer_usage(bufInfo->bufferUsage);
	if (!bufferUsage)
	{
		LOG_ERROR("Invalid buffer usage")
		return;
	}
	
	VmaMemoryUsage memoryUsage = get_memory_usage(bufInfo->memoryUsage);
	if (memoryUsage == VMA_MEMORY_USAGE_UNKNOWN)
	{
		LOG_ERROR("Invalid memory usage (buffer)")
		return;
	}
	if (bufInfo->transferDst)
		bufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	if (bufInfo->transferSrc)
		bufferUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VulkanBuffer* vulkanBuffer = new VulkanBuffer(&_allocator, size, bufferUsage, memoryUsage);
	buffer->data = vulkanBuffer;
	buffer->size = size;
	buffer->type = rhi::Resource::ResourceType::BUFFER;
	buffer->bufferInfo = *bufInfo;
	if (data == nullptr)
		return;
	if (data != nullptr && memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY)
	{
		LOG_ERROR("Can't copy data from CPU to buffer if memory usage is VMA_MEMORY_USAGE_GPU_ONLY")
		return;
	}
	vulkanBuffer->copy_from(&_allocator, data, size);
}

void vulkan::VulkanRHI::update_buffer_data(rhi::Buffer* buffer, uint64_t size, void* data)
{
	if (!data || !buffer || !buffer->data || size == 0)
	{
		LOG_ERROR("Can't use update_buffer_data if buffer, data or size is invalid")
		return;
	}
	
	if (buffer->bufferInfo.memoryUsage == rhi::GPU)
	{
		LOG_ERROR("Can't copy data from CPU to buffer if memory usage is VMA_MEMORY_USAGE_GPU_ONLY")
		return;
	}
	if (!buffer->bufferInfo.transferDst)
	{
		LOG_ERROR("Can't copy because buffer usage doesn't have the transfer dst flag")
		return;
	}

	VulkanBuffer* vulkanBuffer = static_cast<VulkanBuffer*>(buffer->data);
	vulkanBuffer->copy_from(&_allocator, data, size);
}

void vulkan::VulkanRHI::create_texture(rhi::Texture* texture, rhi::TextureInfo* texInfo)
{
	if (!texture)
	{
		LOG_ERROR("Can't create texture if texture parameter is invalid")
		return;
	}

	if (texInfo->format == rhi::UNDEFINED_FORMAT)
	{
		LOG_ERROR("Can't create texture because of undefined format")
		return;
	}
	if (texInfo->textureUsage == rhi::UNDEFINED_USAGE)
	{
		LOG_ERROR("Can't create texture because of undefined usage")
		return;
	}
	if (texInfo->memoryUsage == rhi::UNDEFINED_MEMORY_USAGE)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined memory usage. Failed to create VkImage")
		return;
	}
	if (texInfo->samplesCount == rhi::UNDEFINED_SAMPLE_COUNT)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined memory usage. Failed to create VkImage")
		return;
	}
	if (texInfo->textureDimension == rhi::UNDEFINED_TEXTURE_DIMENSION)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined texture dimension. Failed to create VkImage")
		return;
	}
	
	VmaMemoryUsage memoryUsage = get_memory_usage(texInfo->memoryUsage);
	
	VkImageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.format = get_format(texInfo->format);
	createInfo.arrayLayers = texInfo->layersCount;
	createInfo.mipLevels = texInfo->mipLevels;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.extent = VkExtent3D{ texInfo->width, texInfo->height, 1 };
	createInfo.samples = get_sample_count(texInfo->samplesCount);

	VkImageUsageFlags imgUsage = get_image_usage(texInfo->textureUsage);
	if (texInfo->transferSrc)
		imgUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if (texInfo->transferDst)
		imgUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	createInfo.usage = imgUsage;
	createInfo.imageType = get_image_type(texInfo->textureDimension);
	
	VulkanTexture* vkText = new VulkanTexture(createInfo, &_allocator, memoryUsage);
	if (vkText->_image == VK_NULL_HANDLE)
	{
		LOG_ERROR("Failed to allocate VkImage")
		return;
	}
	texture->data = vkText;
	texture->type = rhi::Resource::ResourceType::TEXTURE;
	texture->textureInfo = *texInfo;
}

void vulkan::VulkanRHI::create_texture_view(rhi::TextureView* textureView, rhi::TextureViewInfo* viewInfo, rhi::Texture* texture)
{
	if (!textureView || !texture)
	{
		LOG_ERROR("Can't create texture view if one of the parameters is invalid")
	}
	
	rhi::TextureInfo texInfo = texture->textureInfo;
	VulkanTexture* vkTexture = static_cast<VulkanTexture*>(texture->data);
	
	VkFormat format = get_format(texInfo.format);
	VkImageUsageFlags imgUsage = get_image_usage(texInfo.textureUsage);

	VkImageAspectFlags aspectFlags;
	if (imgUsage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	else
		aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.image = vkTexture->_image;
	createInfo.format = format;
	createInfo.subresourceRange.baseMipLevel = viewInfo->baseMipLevel;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = viewInfo->baseLayer;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.subresourceRange.aspectMask = aspectFlags;

	VkImageView* view = new VkImageView();
	VK_CHECK(vkCreateImageView(_vulkanDevice.get_device(), &createInfo, nullptr, view));
	textureView->handle = view;
	textureView->viewInfo = *viewInfo;
	textureView->texture = texture;
}

void vulkan::VulkanRHI::create_sampler(rhi::Sampler* sampler, rhi::SamplerInfo* sampInfo)
{
	if (!sampler)
	{
		LOG_ERROR("Can't create sampler if sampler parameter is invalid")
		return;
	}
	if (sampInfo->addressMode == rhi::UNDEFINED_ADDRESS_MODE)
	{
		LOG_ERROR("VulkanRHI::create_sampler(): Undefined address mode. Failed to create VkSampler")
		return;
	}
	if (sampInfo->filter == rhi::UNDEFINED_FILTER)
	{
		LOG_ERROR("VulkanRHI::create_sampler(): Undefined filter. Failed to create VkSampler")
		return;
	}
	if (sampInfo->borderColor == rhi::UNDEFINED_BORDER_COLOR)
	{
		LOG_ERROR("VulkanRHI::create_sampler(): Undefined border color. Failed to create VkSampler")
		return;
	}
	VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	get_filter(sampInfo->filter, createInfo);
	createInfo.addressModeU = get_address_mode(sampInfo->addressMode);
	createInfo.addressModeV = createInfo.addressModeU;
	createInfo.addressModeW = createInfo.addressModeU;
	createInfo.minLod = sampInfo->minLod;
	createInfo.maxLod = sampInfo->maxLod;
	if (createInfo.anisotropyEnable == VK_TRUE)
		createInfo.maxAnisotropy = sampInfo->maxAnisotropy;
	if (sampInfo->borderColor != rhi::UNDEFINED_BORDER_COLOR)
		createInfo.borderColor = get_border_color(sampInfo->borderColor);
	VkSampler* vkSampler = new VkSampler();
	VK_CHECK(vkCreateSampler(_vulkanDevice.get_device(), &createInfo, nullptr, vkSampler));
	sampler->handle = vkSampler;
	sampler->sampInfo = *sampInfo;
}

void vulkan::VulkanRHI::create_graphics_pipeline(rhi::Pipeline* pipeline, rhi::GraphicsPipelineInfo* info)
{
	pipeline->type = rhi::PipelineType::GRAPHICS_PIPELINE;

	if (info->assemblyState.topologyType == rhi::UNDEFINED_TOPOLOGY_TYPE)
	{
		LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined topology type. Failed to create VkPipeline")
		return;
	}
	
	VkPipelineInputAssemblyStateCreateInfo assemblyState{};
	assemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assemblyState.topology = get_primitive_topology(info->assemblyState.topologyType);
	assemblyState.primitiveRestartEnable = VK_FALSE;

	// Only dynamic viewports, no static
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pScissors = nullptr;
	viewportState.scissorCount = 1;
	viewportState.pViewports = nullptr;
	viewportState.viewportCount = 1;

	if (info->rasterizationState.cullMode == rhi::UNDEFINED_CULL_MODE)
	{
		LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined cull mode. Failed to create VkPipeline")
		return;
	}
	if (info->rasterizationState.polygonMode == rhi::UNDEFINED_POLYGON_MODE)
	{
		LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined polygon mode. Failed to create VkPipeline")
		return;
	}
	if (info->rasterizationState.frontFace == rhi::UNDEFINED_FRONT_FACE)
	{
		LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined front face. Failed to create VkPipeline")
		return;
	}

	VkPipelineRasterizationStateCreateInfo rasterizationState{};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = get_polygon_mode(info->rasterizationState.polygonMode);
	rasterizationState.lineWidth = 1.0f;
	rasterizationState.cullMode = get_cull_mode(info->rasterizationState.cullMode);
	rasterizationState.frontFace = get_front_face(info->rasterizationState.frontFace);
	rasterizationState.depthBiasEnable = info->rasterizationState.isBiasEnabled ? VK_TRUE : VK_FALSE;
	rasterizationState.depthBiasConstantFactor = 0.0f;
	rasterizationState.depthBiasClamp = 0.0f;
	rasterizationState.depthBiasSlopeFactor = 0.0f;

	if (info->multisampleState.sampleCount == rhi::UNDEFINED_SAMPLE_COUNT)
	{
		LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined sample count. Failed to create VkPipeline")
		return;
	}

	VkPipelineMultisampleStateCreateInfo multisampleState{};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.sampleShadingEnable = info->multisampleState.isEnabled ? VK_TRUE : VK_FALSE;
	multisampleState.rasterizationSamples = get_sample_count(info->multisampleState.sampleCount);
	multisampleState.minSampleShading = 1.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = info->multisampleState.isEnabled ? VK_TRUE : VK_FALSE;
	multisampleState.alphaToOneEnable = info->multisampleState.isEnabled ? VK_TRUE : VK_FALSE;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	for (auto& desc : info->bindingDescriptrions)
	{
		VkVertexInputBindingDescription description;
		description.binding = desc.binding;
		description.stride = desc.stride;
		description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDescriptions.push_back(description);
	}

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	for (auto& desc : info->attributeDescriptions)
	{
		if (desc.format == rhi::UNDEFINED_FORMAT)
		{
			LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined format. Failed to create VkPipeline")
			return;
		}
		VkVertexInputAttributeDescription description;
		description.binding = desc.binding;
		description.format = get_format(desc.format);
		description.location = desc.location;
		description.offset = desc.offset;
		attributeDescriptions.push_back(description);
	}

	VkPipelineVertexInputStateCreateInfo inputState{};
	inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	inputState.pVertexBindingDescriptions = bindingDescriptions.data();
	inputState.vertexBindingDescriptionCount = bindingDescriptions.size();
	inputState.pVertexAttributeDescriptions = attributeDescriptions.data();
	inputState.vertexAttributeDescriptionCount = attributeDescriptions.size();

	std::vector<VkDynamicState> dynamicStates;
	dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	if (info->rasterizationState.isBiasEnabled)
		dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	for (auto& attach : info->colorBlendState.colorBlendAttachments)
	{
		VkPipelineColorBlendAttachmentState attachmentState{};
		attachmentState.colorWriteMask = (VkColorComponentFlags)attach.colorWriteMask;
		if (attach.isBlendEnabled)
		{
			if (attach.srcColorBlendFactor == rhi::UNDEFINED_BLEND_FACTOR)
			{
				LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined src color blend factor. Failed to create VkPipeline")
				return;
			}
			if (attach.dstColorBlendFactor == rhi::UNDEFINED_BLEND_FACTOR)
			{
				LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined dst color blend factor. Failed to create VkPipeline")
				return;
			}
			if (attach.srcAlphaBlendFactor == rhi::UNDEFINED_BLEND_FACTOR)
			{
				LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined src alpha blend factor. Failed to create VkPipeline")
				return;
			}
			if (attach.dstAlphaBlendFactor == rhi::UNDEFINED_BLEND_FACTOR)
			{
				LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined dst alpha blend factor. Failed to create VkPipeline")
				return;
			}
			if (attach.colorBlendOp == rhi::UNDEFINED_BLEND_OP)
			{
				LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined color blend op. Failed to create VkPipeline")
				return;
			}
			if (attach.alphaBlendOp == rhi::UNDEFINED_BLEND_OP)
			{
				LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined alpha blend op. Failed to create VkPipeline")
				return;
			}
			attachmentState.blendEnable = VK_TRUE;
			attachmentState.srcColorBlendFactor = get_blend_factor(attach.srcColorBlendFactor);
			attachmentState.dstColorBlendFactor = get_blend_factor(attach.dstColorBlendFactor);
			attachmentState.srcAlphaBlendFactor = get_blend_factor(attach.srcAlphaBlendFactor);
			attachmentState.dstAlphaBlendFactor = get_blend_factor(attach.dstAlphaBlendFactor);
			attachmentState.colorBlendOp = get_blend_op(attach.colorBlendOp);
			attachmentState.alphaBlendOp = get_blend_op(attach.alphaBlendOp);
		}
		attachmentState.blendEnable = VK_FALSE;
		colorBlendAttachments.push_back(attachmentState);
	}

	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.attachmentCount = colorBlendAttachments.size();
	colorBlendState.pAttachments = colorBlendAttachments.data();
	colorBlendState.logicOpEnable = info->colorBlendState.isLogicOpEnabled ? VK_TRUE : VK_FALSE;
	colorBlendState.logicOp = get_logic_op(info->colorBlendState.logicOp);

	VulkanShaderStages shaderStages;
	std::vector<VkPipelineShaderStageCreateInfo> pipelineStages; 
	for (auto& shader : info->shaderStages)
	{
		VulkanShader* vulkanShader = static_cast<VulkanShader*>(shader.handle);
		shaderStages.add_stage(vulkanShader, (VkShaderStageFlagBits)get_shader_stage(shader.type));
		VkPipelineShaderStageCreateInfo shaderStage{};
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.module = vulkanShader->get_shader_module();
		shaderStage.stage = (VkShaderStageFlagBits)get_shader_stage(shader.type);
		shaderStage.pName = "main";
		shaderStage.pSpecializationInfo = nullptr;
		pipelineStages.push_back(shaderStage);
	}

	if (info->depthStencilState.compareOp == rhi::UNDEFINED_COMPARE_OP)
	{
		LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined compare op, depth. Failed to create VkPipeline")
		return;
	}
	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = info->depthStencilState.isDepthTestEnabled ? VK_TRUE : VK_FALSE;
	depthStencilState.depthWriteEnable = info->depthStencilState.isDepthWriteEnabled ? VK_TRUE : VK_FALSE;
	depthStencilState.depthCompareOp = get_compare_op(info->depthStencilState.compareOp);
	depthStencilState.minDepthBounds = 0.0f;
	depthStencilState.maxDepthBounds = 1.0f;
	depthStencilState.stencilTestEnable = VK_FALSE;
	std::vector<rhi::StencilOpState> stencilOps = { info->depthStencilState.backStencil, info->depthStencilState.frontStencil };
	if (info->depthStencilState.isStencilTestEnabled)
	{
		depthStencilState.stencilTestEnable = VK_TRUE;

		std::vector<VkStencilOpState> vkStencilStates;
		for (auto& stencilState : stencilOps)
		{
			if (stencilState.compareOp == rhi::UNDEFINED_COMPARE_OP)
			{
				LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined compare op, stencil. Failed to create VkPipeline")
				return;
			}
			if (stencilState.failOp == rhi::UNDEFINED_STENCIL_OP)
			{
				LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined fail op stencil. Failed to create VkPipeline")
				return;
			}
			if (stencilState.passOp == rhi::UNDEFINED_STENCIL_OP)
			{
				LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined pass op stencil. Failed to create VkPipeline")
				return;
			}
			if (stencilState.depthFailOp == rhi::UNDEFINED_STENCIL_OP)
			{
				LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Undefined depth fail op stencil. Failed to create VkPipeline")
				return;
			}
			VkStencilOpState stencilOp{};
			stencilOp.failOp = get_stencil_op(stencilState.failOp);
			stencilOp.passOp = get_stencil_op(stencilState.passOp);
			stencilOp.depthFailOp = get_stencil_op(stencilState.depthFailOp);
			stencilOp.compareOp = get_compare_op(stencilState.compareOp);
			stencilOp.compareMask = stencilState.compareMask;
			stencilOp.writeMask = stencilState.writeMask;
			stencilOp.reference = stencilState.reference;
			vkStencilStates.push_back(stencilOp);
		}
	}

	VkPipelineLayout pipelineLayout = shaderStages.get_pipeline_layout(_vulkanDevice.get_device());

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = *static_cast<VkRenderPass*>(info->renderPass.handle);
	pipelineInfo.subpass = 0;
	pipelineInfo.stageCount = pipelineStages.size();
	pipelineInfo.pStages = pipelineStages.data();
	pipelineInfo.pVertexInputState = &inputState;
	pipelineInfo.pInputAssemblyState = &assemblyState;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizationState;
	pipelineInfo.pMultisampleState = &multisampleState;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.pColorBlendState = &colorBlendState;
	pipelineInfo.pDepthStencilState = &depthStencilState;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline* vkPipeline = new VkPipeline();
	VK_CHECK(vkCreateGraphicsPipelines(_vulkanDevice.get_device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, vkPipeline));

	pipeline->handle = vkPipeline;
}

void vulkan::VulkanRHI::create_render_pass(rhi::RenderPass* renderPass, rhi::RenderPassInfo* passInfo)
{
	std::vector<VkAttachmentDescription> attachDescriptions;
	std::vector<VkAttachmentReference> colorAttachRefs;
	VkAttachmentReference depthAttachRef;
	bool oneDepth = false;

	if (passInfo->renderTargets.empty())
	{
		LOG_ERROR("VulkanRHI::create_render_pass(): Failed to create VkRenderPass because there are no render targets")
		return;
	}
	
	for (auto& target: passInfo->renderTargets)
	{
		rhi::TextureInfo texInfo = target.target->texture->textureInfo;
		VkAttachmentDescription attachInfo{};
		attachInfo.format = get_format(texInfo.format);
		attachInfo.samples = get_sample_count(texInfo.samplesCount);
		attachInfo.loadOp = get_attach_load_op(target.loadOp);
		attachInfo.storeOp = get_attach_store_op(target.storeOp);
		attachInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachInfo.initialLayout = get_image_layout(target.initialLayout);
		attachInfo.finalLayout = get_image_layout(target.finalLayout);
		attachDescriptions.push_back(attachInfo);

		VkAttachmentReference attachRef{};
		attachRef.attachment = attachDescriptions.size() - 1;
		attachRef.layout = get_image_layout(target.renderPassLayout);

		if (target.type == rhi::RENDER_TARGET_DEPTH && !oneDepth)
		{
			depthAttachRef = attachRef;
			oneDepth = true;
		}
		else if (target.type == rhi::RENDER_TARGET_DEPTH && oneDepth)
		{
			LOG_WARNING("VulkanRHI::create_render_pass(): There are more than one depth attachment. Old one will be overwritten")
			depthAttachRef = attachRef;
		}
		else
		{
			colorAttachRefs.push_back(attachRef);
		}
	}

	VkSubpassDescription subpass{};
	if (passInfo->pipelineType == rhi::COMPUTE_PIPELINE || passInfo->pipelineType == rhi::UNDEFINED_PIPELINE_TYPE)
	{
		LOG_ERROR("VulkanRHI::create_render_pass(): Invalid pipeline type")
		return;
	}

	subpass.pipelineBindPoint = get_pipeline_bind_point(passInfo->pipelineType);
	if (!colorAttachRefs.empty())
	{
		subpass.colorAttachmentCount = colorAttachRefs.size();
		subpass.pColorAttachments = colorAttachRefs.data();
	}
	if (oneDepth)
		subpass.pDepthStencilAttachment = &depthAttachRef;

	std::vector<VkSubpassDependency> dependencies;
	if (!colorAttachRefs.empty())
	{
		VkSubpassDependency colorDependency{};
		colorDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		colorDependency.dstSubpass = 0;
		colorDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		colorDependency.srcAccessMask = 0;
		colorDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		colorDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies.push_back(colorDependency);
	}
	if (oneDepth)
	{
		VkSubpassDependency depthDependency{};
		depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		depthDependency.dstSubpass = 0;
		depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depthDependency.srcAccessMask = 0;
		depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies.push_back(depthDependency);
	}

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachDescriptions.size();
	renderPassInfo.pAttachments = attachDescriptions.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = dependencies.size();
	renderPassInfo.pDependencies = dependencies.data();

	VkRenderPass vkRenderPass;
	VK_CHECK(vkCreateRenderPass(_vulkanDevice.get_device(), &renderPassInfo, nullptr, &vkRenderPass));
	VkFramebuffer framebuffer = create_framebuffer(vkRenderPass, passInfo->renderTargets);
	VulkanRenderPass* vkPass = new VulkanRenderPass();
	vkPass->renderPass = vkRenderPass;
	vkPass->framebuffer = framebuffer;
	renderPass->handle = vkPass;
}

// private methods
vkb::Instance vulkan::VulkanRHI::create_instance()
{
	LOG_INFO("Start creating Vulkan instance")
	vkb::InstanceBuilder builder;
	builder.set_app_name("AdAstris Engine");
	builder.require_api_version(1, 3, 0);
#ifndef VK_RELEASE
	builder.use_default_debug_messenger();
	builder.request_validation_layers(true);
#else
	builder.request_validation_layers(false);
#endif
	LOG_INFO("Finish creating Vulkan instance")
	return builder.build().value();
}

void vulkan::VulkanRHI::create_allocator()
{
	LOG_INFO("Start creating allocator")
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = _vulkanDevice.get_physical_device();
	allocatorInfo.device = _vulkanDevice.get_device();
	allocatorInfo.instance = _instance;
	vmaCreateAllocator(&allocatorInfo, &_allocator);
	LOG_INFO("End creating allocator")
}

VkFramebuffer vulkan::VulkanRHI::create_framebuffer(VkRenderPass renderPass, std::vector<rhi::RenderTarget>& renderTargets)
{
	std::vector<VkImageView> attachViews;
	for (auto& target : renderTargets)
	{
		VkImageView view = *static_cast<VkImageView*>(target.target->handle);
		attachViews.push_back(view);
	}
	
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.layers = 1;
	framebufferInfo.width = renderTargets[0].target->texture->textureInfo.width;
	framebufferInfo.height = renderTargets[0].target->texture->textureInfo.height;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.pAttachments = attachViews.data();
	framebufferInfo.attachmentCount = attachViews.size();

	VkFramebuffer framebuffer;
	VK_CHECK(vkCreateFramebuffer(_vulkanDevice.get_device(), &framebufferInfo, nullptr, &framebuffer));
	return framebuffer;
}

VkFormat vulkan::get_format(rhi::Format format)
{
	switch (format)
	{
		case rhi::UNDEFINED_FORMAT:
			return VK_FORMAT_UNDEFINED;
		case rhi::R4G4_UNORM:
			return VK_FORMAT_R4G4_UNORM_PACK8;
		case rhi::R4G4B4A4_UNORM:
			return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
		case rhi::B4G4R4A4_UNORM:
			return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
		case rhi::R5G5B5A1_UNORM:
			return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
		case rhi::B5G5R5A1_UNORM:
			return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
		case rhi::A1R5G5B5_UNORM:
			return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
			
		case rhi::R8_UNORM:
			return VK_FORMAT_R8_UNORM;
		case rhi::R8_SNORM:
			return VK_FORMAT_R8_SNORM;
		case rhi::R8_UINT:
			return VK_FORMAT_R8_UINT;
		case rhi::R8_SINT:
			return VK_FORMAT_R8_SINT;
		case rhi::R8_SRGB:
			return VK_FORMAT_R8_SRGB;
			
		case rhi::R8G8_UNORM:
			return VK_FORMAT_R8G8_UNORM;
		case rhi::R8G8_SNORM:
			return VK_FORMAT_R8G8_SNORM;
		case rhi::R8G8_UINT:
			return VK_FORMAT_R8G8_UINT;
		case rhi::R8G8_SINT:
			return VK_FORMAT_R8G8_SINT;
		case rhi::R8G8_SRGB:
			return VK_FORMAT_R8G8_SRGB;

		case rhi::R8G8B8A8_UNORM:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case rhi::R8G8B8A8_SNORM:
			return VK_FORMAT_R8G8B8A8_SNORM;
		case rhi::R8G8B8A8_UINT:
			return VK_FORMAT_R8G8B8A8_UINT;
		case rhi::R8G8B8A8_SINT:
			return VK_FORMAT_R8G8B8A8_SINT;
		case rhi::R8G8B8A8_SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;

		case rhi::B8G8R8A8_SRGB:
			return VK_FORMAT_B8G8R8A8_SRGB;
		case rhi::B8G8R8A8_UNORM:
			return VK_FORMAT_B8G8R8A8_UNORM;
		case rhi::B8G8R8A8_SNORM:
			return VK_FORMAT_B8G8R8A8_SNORM;

		case rhi::R10G10B10A2_UNORM:
			return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		case rhi::R10G10B10A2_SNORM:
			return VK_FORMAT_A2R10G10B10_SNORM_PACK32;

		case rhi::R16_UNORM:
			return VK_FORMAT_R16_UNORM;
		case rhi::R16_SNORM:
			return VK_FORMAT_R16_SNORM;
		case rhi::R16_UINT:
			return VK_FORMAT_R16_UINT;
		case rhi::R16_SINT:
			return VK_FORMAT_R16_SINT;
		case rhi::R16_SFLOAT:
			return VK_FORMAT_R16_SFLOAT;

		case rhi::R16G16_UNORM:
			return VK_FORMAT_R16G16_UNORM;
		case rhi::R16G16_SNORM:
			return VK_FORMAT_R16G16_SNORM;
		case rhi::R16G16_UINT:
			return VK_FORMAT_R16G16_UINT;
		case rhi::R16G16_SINT:
			return VK_FORMAT_R16G16_SINT;
		case rhi::R16G16_SFLOAT:
			return VK_FORMAT_R16G16_SFLOAT;

		case rhi::R16G16B16A16_UNORM:
			return VK_FORMAT_R16G16B16A16_UNORM;
		case rhi::R16G16B16A16_SNORM:
			return VK_FORMAT_R16G16B16A16_SNORM;
		case rhi::R16G16B16A16_UINT:
			return VK_FORMAT_R16G16B16A16_UINT;
		case rhi::R16G16B16A16_SINT:
			return VK_FORMAT_R16G16B16A16_SINT;
		case rhi::R16G16B16A16_SFLOAT:
			return VK_FORMAT_R16G16B16A16_SFLOAT;

		case rhi::R32_UINT:
			return VK_FORMAT_R32_UINT;
		case rhi::R32_SINT:
			return VK_FORMAT_R32_SINT;
		case rhi::R32_SFLOAT:
			return VK_FORMAT_R32_SFLOAT;
		case rhi::R32G32_UINT:
			return VK_FORMAT_R32G32_UINT;
		case rhi::R32G32_SINT:
			return VK_FORMAT_R32G32_SINT;
		case rhi::R32G32_SFLOAT:
			return VK_FORMAT_R32G32_SFLOAT;

		case rhi::R32G32B32_UINT:
			return VK_FORMAT_R32G32B32_UINT;
		case rhi::R32G32B32_SINT:
			return VK_FORMAT_R32G32B32_SINT;
		case rhi::R32G32B32_SFLOAT:
			return VK_FORMAT_R32G32B32_SFLOAT;

		case rhi::R32G32B32A32_UINT:
			return VK_FORMAT_R32G32B32A32_UINT;
		case rhi::R32G32B32A32_SINT:
			return VK_FORMAT_R32G32B32A32_SINT;
		case rhi::R32G32B32A32_SFLOAT:
			return VK_FORMAT_R32G32B32A32_SFLOAT;

		case rhi::D16_UNORM:
			return VK_FORMAT_D16_UNORM;
		case rhi::D32_SFLOAT:
			return VK_FORMAT_D32_SFLOAT;

		case rhi::S8_UINT:
			return VK_FORMAT_S8_UINT;
		case rhi::D16_UNORM_S8_UINT:
			return VK_FORMAT_D16_UNORM_S8_UINT;
		case rhi::D24_UNORM_S8_UINT:
			return VK_FORMAT_D24_UNORM_S8_UINT;
		case rhi::D32_SFLOAT_S8_UINT:
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
	}

	return VK_FORMAT_UNDEFINED;
}

VkSampleCountFlagBits vulkan::get_sample_count(rhi::SampleCount sampleCount)
{
	switch (sampleCount)
	{
		case rhi::SAMPLE_COUNT_1_BIT:
			return VK_SAMPLE_COUNT_1_BIT;
		case rhi::SAMPLE_COUNT_2_BIT:
			return VK_SAMPLE_COUNT_2_BIT;
		case rhi::SAMPLE_COUNT_4_BIT:
			return VK_SAMPLE_COUNT_4_BIT;
		case rhi::SAMPLE_COUNT_8_BIT:
			return VK_SAMPLE_COUNT_8_BIT;
		case rhi::SAMPLE_COUNT_16_BIT:
			return VK_SAMPLE_COUNT_16_BIT;
		case rhi::SAMPLE_COUNT_32_BIT:
			return VK_SAMPLE_COUNT_32_BIT;
		case rhi::SAMPLE_COUNT_64_BIT:
			return VK_SAMPLE_COUNT_64_BIT;
	}
}

VmaMemoryUsage vulkan::get_memory_usage(rhi::MemoryUsage memoryUsage)
{
	switch (memoryUsage)
	{
		case rhi::UNDEFINED_MEMORY_USAGE:
			LOG_ERROR("Undefined memory usage")
			return VMA_MEMORY_USAGE_UNKNOWN;
		case rhi::CPU:
			return VMA_MEMORY_USAGE_CPU_ONLY;
		case rhi::GPU:
			return VMA_MEMORY_USAGE_GPU_ONLY;
		case rhi::CPU_TO_GPU:
			return VMA_MEMORY_USAGE_CPU_TO_GPU;
	}
}

VkImageType vulkan::get_image_type(rhi::TextureDimension dimension)
{
	switch (dimension)
	{
		case rhi::TEXTURE1D:
			return VK_IMAGE_TYPE_1D;
		case rhi::TEXTURE2D:
			return VK_IMAGE_TYPE_2D;
		case rhi::TEXTURE3D:
			return VK_IMAGE_TYPE_3D;
	}
}


void vulkan::get_filter(rhi::Filter filter, VkSamplerCreateInfo& samplerInfo)
{
	switch (filter)
	{
		case rhi::MIN_MAG_MIP_NEAREST:
		case rhi::MINIMUM_MIN_MAG_MIP_NEAREST:
		case rhi::MAXIMUM_MIN_MAG_MIP_NEAREST:
		case rhi::COMPARISON_MIN_MAG_MIP_NEAREST:
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case rhi::MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::MINIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::MAXIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::COMPARISON_MIN_MAG_NEAREST_MIP_LINEAR:
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case rhi::MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::MINIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::MAXIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::COMPARISON_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case rhi::MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::MINIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::MAXIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::COMPARISON_MIN_NEAREST_MAG_MIP_LINEAR:
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case rhi::MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::MINIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::MAXIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::COMPARISON_MIN_LINEAR_MAG_MIP_NEAREST:
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case rhi::MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::MINIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::MAXIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::COMPARISON_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case rhi::MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::MINIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::MAXIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::COMPARISON_MIN_MAG_LINEAR_MIP_NEAREST:
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case rhi::MIN_MAG_MIP_LINEAR:
		case rhi::MINIMUM_MIN_MAG_MIP_LINEAR:
		case rhi::MAXIMUM_MIN_MAG_MIP_LINEAR:
		case rhi::COMPARISON_MIN_MAG_MIP_LINEAR:
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case rhi::ANISOTROPIC:
		case rhi::MINIMUM_ANISOTROPIC:
		case rhi::MAXIMUM_ANISOTROPIC:
		case rhi::COMPARISON_ANISOTROPIC:
			// have to think about this
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.anisotropyEnable = VK_TRUE;
			break;
		default:
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
	}
}

VkBorderColor vulkan::get_border_color(rhi::BorderColor borderColor)
{
	switch (borderColor)
	{
		case rhi::FLOAT_TRANSPARENT_BLACK:
			return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		case rhi::INT_TRANSPARENT_BLACK:
			return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
		case rhi::FLOAT_OPAQUE_BLACK:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		case rhi::INT_OPAQUE_BLACK:
			return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		case rhi::FLOAT_OPAQUE_WHITE:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		case rhi::INT_OPAQUE_WHITE:
			return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	}
}

VkSamplerAddressMode vulkan::get_address_mode(rhi::AddressMode addressMode)
{
	switch (addressMode)
	{
		case rhi::REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case rhi::MIRRORED_REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case rhi::CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case rhi::CLAMP_TO_BORDER:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case rhi::MIRROR_CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
	}
}

VkBufferUsageFlags vulkan::get_buffer_usage(rhi::ResourceUsage usage)
{
	switch (usage)
	{
		case rhi::SAMPLED_TEXTURE:
		case rhi::STORAGE_TEXTURE:
		case rhi::COLOR_ATTACHMENT:
		case rhi::DEPTH_STENCIL_ATTACHMENT:
		case rhi::TRANSIENT_ATTACHMENT:
		case rhi::INPUT_ATTACHMENT:
		case rhi::UNDEFINED_USAGE:
			LOG_ERROR("Invalid usage. Can't use buffer. 0 will be returned")
			return 0;
		case rhi::UNIFORM_TEXEL_BUFFER:
			return VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		case rhi::STORAGE_TEXEL_BUFFER:
			return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		case rhi::UNIFORM_BUFFER:
			return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		case rhi::STORAGE_BUFFER:
			return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		case rhi::INDEX_BUFFER:
			return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		case rhi::VERTEX_BUFFER:
			return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		case rhi::INDIRECT_BUFFER:
			return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	}
}

VkImageUsageFlags vulkan::get_image_usage(rhi::ResourceUsage usage)
{
	switch (usage)
	{
		case rhi::UNIFORM_TEXEL_BUFFER:
		case rhi::STORAGE_TEXEL_BUFFER:
		case rhi::UNIFORM_BUFFER:
		case rhi::STORAGE_BUFFER:
		case rhi::INDEX_BUFFER:
		case rhi::VERTEX_BUFFER:
		case rhi::INDIRECT_BUFFER:
		case rhi::UNDEFINED_USAGE:
			LOG_ERROR("Invalid usage. Can't use image. 0 will be returned")
			return 0;
		case rhi::SAMPLED_TEXTURE:
			return VK_IMAGE_USAGE_SAMPLED_BIT;
		case rhi::STORAGE_TEXTURE:
			return VK_IMAGE_USAGE_STORAGE_BIT;
		case rhi::COLOR_ATTACHMENT:
			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		case rhi::DEPTH_STENCIL_ATTACHMENT:
			return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		case rhi::TRANSIENT_ATTACHMENT:
			return VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		case rhi::INPUT_ATTACHMENT:
			return VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	}
}

VkPrimitiveTopology vulkan::get_primitive_topology(rhi::TopologyType topologyType)
{
	switch (topologyType)
	{
		case rhi::TOPOLOGY_POINT:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case rhi::TOPOLOGY_LINE:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case rhi::TOPOLOGY_TRIANGLE:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case rhi::TOPOLOGY_PATCH:
			return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
	}
}

VkPolygonMode vulkan::get_polygon_mode(rhi::PolygonMode polygonMode)
{
	switch (polygonMode)
	{
		case rhi::POLYGON_MODE_FILL:
			return VK_POLYGON_MODE_FILL;
		case rhi::POLYGON_MODE_LINE:
			return VK_POLYGON_MODE_LINE;
		case rhi::POLYGON_MODE_POINT:
			return VK_POLYGON_MODE_POINT;
	}
}

VkCullModeFlags vulkan::get_cull_mode(rhi::CullMode cullMode)
{
	switch (cullMode)
	{
		case rhi::CULL_MODE_NONE:
			return VK_CULL_MODE_NONE;
		case rhi::CULL_MODE_FRONT:
			return VK_CULL_MODE_FRONT_BIT;
		case rhi::CULL_MODE_BACK:
			return VK_CULL_MODE_BACK_BIT;
		case rhi::CULL_MODE_FRONT_AND_BACK:
			return VK_CULL_MODE_FRONT_AND_BACK;
	}
}

VkFrontFace vulkan::get_front_face(rhi::FrontFace frontFace)
{
	switch (frontFace)
	{
		case rhi::FRONT_FACE_CLOCKWISE:
			return VK_FRONT_FACE_CLOCKWISE;
		case rhi::FRONT_FACE_COUNTER_CLOCKWISE:
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}
}

VkShaderStageFlags vulkan::get_shader_stage(rhi::ShaderType shaderType)
{
	switch (shaderType)
	{
		case rhi::UNDEFINED_SHADER_TYPE:
			LOG_ERROR("Undefined shader type")
			return 0;
		case rhi::VERTEX:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case rhi::FRAGMENT:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case rhi::TESSELLATION_CONTROL:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case rhi::TESSELLATION_EVALUATION:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case rhi::GEOMETRY:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		case rhi::COMPUTE:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		case rhi::MESH:
			return VK_SHADER_STAGE_MESH_BIT_NV;
		case rhi::TASK:
			return VK_SHADER_STAGE_TASK_BIT_NV;
		case rhi::RAY_GENERATION:
			return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		case rhi::RAY_INTERSECTION:
			return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
		case rhi::RAY_ANY_HIT:
			return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		case rhi::RAY_CLOSEST_HIT:
			return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		case rhi::RAY_MISS:
			return VK_SHADER_STAGE_MISS_BIT_KHR;
		case rhi::RAY_CALLABLE:
			return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
	}
}

VkLogicOp vulkan::get_logic_op(rhi::LogicOp logicOp)
{
	switch (logicOp)
	{
		case rhi::LOGIC_OP_CLEAR:
			return VK_LOGIC_OP_CLEAR;
		case rhi::LOGIC_OP_AND:
			return VK_LOGIC_OP_AND;
		case rhi::LOGIC_OP_AND_REVERSE:
			return VK_LOGIC_OP_AND_REVERSE;
		case rhi::LOGIC_OP_COPY:
			return VK_LOGIC_OP_COPY;
		case rhi::LOGIC_OP_AND_INVERTED:
			return VK_LOGIC_OP_AND_INVERTED;
		case rhi::LOGIC_OP_NO_OP:
			return VK_LOGIC_OP_NO_OP;
		case rhi::LOGIC_OP_XOR:
			return VK_LOGIC_OP_XOR;
		case rhi::LOGIC_OP_OR:
			return VK_LOGIC_OP_OR;
		case rhi::LOGIC_OP_NOR:
			return VK_LOGIC_OP_NOR;
		case rhi::LOGIC_OP_EQUIVALENT:
			return VK_LOGIC_OP_EQUIVALENT;
		case rhi::LOGIC_OP_INVERT:
			return VK_LOGIC_OP_INVERT;
		case rhi::LOGIC_OP_OR_REVERSE:
			return VK_LOGIC_OP_OR_REVERSE;
		case rhi::LOGIC_OP_COPY_INVERTED:
			return VK_LOGIC_OP_COPY_INVERTED;
		case rhi::LOGIC_OP_OR_INVERTED:
			return VK_LOGIC_OP_OR_INVERTED;
		case rhi::LOGIC_OP_NAND:
			return VK_LOGIC_OP_NAND;
		case rhi::LOGIC_OP_SET:
			return VK_LOGIC_OP_SET;
	}
}

VkBlendFactor vulkan::get_blend_factor(rhi::BlendFactor blendFactor)
{
	switch (blendFactor)
	{
		case rhi::BLEND_FACTOR_ZERO:
			return VK_BLEND_FACTOR_ZERO;
		case rhi::BLEND_FACTOR_ONE:
			return VK_BLEND_FACTOR_ONE;
		case rhi::BLEND_FACTOR_SRC_COLOR:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case rhi::BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case rhi::BLEND_FACTOR_DST_COLOR:
			return VK_BLEND_FACTOR_DST_COLOR;
		case rhi::BLEND_FACTOR_ONE_MINUS_DST_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case rhi::BLEND_FACTOR_SRC_ALPHA:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case rhi::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case rhi::BLEND_FACTOR_DST_ALPHA:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case rhi::BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case rhi::BLEND_FACTOR_CONSTANT_COLOR:
			return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case rhi::BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case rhi::BLEND_FACTOR_CONSTANT_ALPHA:
			return VK_BLEND_FACTOR_CONSTANT_ALPHA;
		case rhi::BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		case rhi::BLEND_FACTOR_SRC_ALPHA_SATURATE:
			return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		case rhi::BLEND_FACTOR_SRC1_COLOR:
			return VK_BLEND_FACTOR_SRC1_COLOR;
		case rhi::BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case rhi::BLEND_FACTOR_SRC1_ALPHA:
			return VK_BLEND_FACTOR_SRC1_ALPHA;
		case rhi::BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
	}
}

VkBlendOp vulkan::get_blend_op(rhi::BlendOp blendOp)
{
	switch (blendOp)
	{
		case rhi::BLEND_OP_ADD:
			return VK_BLEND_OP_ADD;
		case rhi::BLEND_OP_SUBTRACT:
			return VK_BLEND_OP_SUBTRACT;
		case rhi::BLEND_OP_REVERSE_SUBTRACT:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case rhi::BLEND_OP_MIN:
			return VK_BLEND_OP_MIN;
		case rhi::BLEND_OP_MAX:
			return VK_BLEND_OP_MAX;
	}
}

VkCompareOp vulkan::get_compare_op(rhi::CompareOp compareOp)
{
	switch (compareOp)
	{
		case rhi::COMPARE_OP_NEVER:
			return VK_COMPARE_OP_NEVER;
		case rhi::COMPARE_OP_LESS:
			return VK_COMPARE_OP_LESS;
		case rhi::COMPARE_OP_EQUAL:
			return VK_COMPARE_OP_EQUAL;
		case rhi::COMPARE_OP_LESS_OR_EQUAL:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case rhi::COMPARE_OP_GREATER:
			return VK_COMPARE_OP_GREATER;
		case rhi::COMPARE_OP_NOT_EQUAL:
			return VK_COMPARE_OP_NOT_EQUAL;
		case rhi::COMPARE_OP_GREATER_OR_EQUAL:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case rhi::COMPARE_OP_ALWAYS:
			return VK_COMPARE_OP_ALWAYS;
	}
}
VkStencilOp vulkan::get_stencil_op(rhi::StencilOp stencilOp)
{
	switch (stencilOp)
	{
		case rhi::STENCIL_OP_KEEP:
			return VK_STENCIL_OP_KEEP;
		case rhi::STENCIL_OP_ZERO:
			return VK_STENCIL_OP_ZERO;
		case rhi::STENCIL_OP_REPLACE:
			return VK_STENCIL_OP_REPLACE;
		case rhi::STENCIL_OP_INCREMENT_AND_CLAMP:
			return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case rhi::STENCIL_OP_DECREMENT_AND_CLAMP:
			return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case rhi::STENCIL_OP_INVERT:
			return VK_STENCIL_OP_INVERT;
		case rhi::STENCIL_OP_INCREMENT_AND_WRAP:
			return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case rhi::STENCIL_OP_DECREMENT_AND_WRAP:
			return VK_STENCIL_OP_DECREMENT_AND_WRAP;
	}
}

VkAttachmentLoadOp vulkan::get_attach_load_op(rhi::LoadOp loadOp)
{
	switch (loadOp)
	{
		case rhi::LOAD_OP_LOAD:
			return VK_ATTACHMENT_LOAD_OP_LOAD;
		case rhi::LOAD_OP_CLEAR:
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case rhi::LOAD_OP_DONT_CARE:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
}

VkAttachmentStoreOp vulkan::get_attach_store_op(rhi::StoreOp storeOp)
{
	switch (storeOp)
	{
		case rhi::STORE_OP_STORE:
			return VK_ATTACHMENT_STORE_OP_STORE;
		case rhi::STORE_OP_DONT_CARE:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
}

VkImageLayout vulkan::get_image_layout(rhi::ResourceLayout resourceLayout)
{
	switch (resourceLayout)
	{
		case rhi::RESOURCE_LAYOUT_UNDEFINED:
			return VK_IMAGE_LAYOUT_UNDEFINED;
		case rhi::RESOURCE_LAYOUT_GENERAL:
			return VK_IMAGE_LAYOUT_GENERAL;
		case rhi::RESOURCE_LAYOUT_SHADER_READ_ONLY:
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case rhi::RESOURCE_LAYOUT_TRANSFER_SRC:
			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		case rhi::RESOURCE_LAYOUT_TRANSFER_DST:
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case rhi::RESOURCE_LAYOUT_COLOR_ATTACHMENT:
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case rhi::RESOURCE_LAYOUT_DEPTH_STENCIL:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		case rhi::RESOURCE_LAYOUT_DEPTH_STENCIL_READ_ONLY:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	}
}

VkPipelineBindPoint vulkan::get_pipeline_bind_point(rhi::PipelineType pipelineType)
{
	switch (pipelineType)
	{
		case rhi::GRAPHICS_PIPELINE:
			return VK_PIPELINE_BIND_POINT_GRAPHICS;
		case rhi::RAY_TRACING_PIPELINE:
			return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
	}
}
