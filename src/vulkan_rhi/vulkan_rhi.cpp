#include "vulkan_rhi.h"
#include "vulkan_common.h"
#include "vulkan_buffer.h"
#include "vulkan_texture.h"
#include "vulkan_shader.h"
#include "vulkan_swap_chain.h"
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

void vulkan::VulkanRHI::create_swap_chain(rhi::SwapChain* swapChain, rhi::SwapChainInfo* info)
{
	if (!swapChain || !info)
	{
		LOG_ERROR("VulkanRHI::create_swap_chain(): Invalid pointers")
		return;
	}
	swapChain->handle = new VulkanSwapChain(info, &_vulkanDevice);
}

void vulkan::VulkanRHI::destroy_swap_chain(rhi::SwapChain* swapChain)
{
	if (!swapChain)
	{
		LOG_ERROR("VulkanRHI::destroy_swap_chain(): Invalid pointer to rhi::SwapChain")
		return;
	}
	VulkanSwapChain* vkSwapChain = static_cast<VulkanSwapChain*>(swapChain->handle);
	delete vkSwapChain;
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
		LOG_ERROR("VulkanRHI::create_texture(): Invalid rhi::Texture pointer")
		return;
	}

	if (texInfo->format == rhi::UNDEFINED_FORMAT)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined format")
		return;
	}
	if (texInfo->textureUsage == rhi::UNDEFINED_USAGE)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined texture usage.")
		return;
	}
	if (texInfo->memoryUsage == rhi::UNDEFINED_MEMORY_USAGE)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined memory usage.")
		return;
	}
	if (texInfo->samplesCount == rhi::UNDEFINED_SAMPLE_COUNT)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined sample count.")
		return;
	}
	if (texInfo->textureDimension == rhi::UNDEFINED_TEXTURE_DIMENSION)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined texture dimension.")
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

	if (texInfo->resourceFlags == rhi::CUBE_TEXTURE)
		createInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

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
		LOG_ERROR("VulkanRHI::create_texture(): Failed to allocate VkImage")
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

	if (texInfo.textureDimension == rhi::TEXTURE1D)
	{
		if (texInfo.layersCount == 1)
		{
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
		}
		else
		{
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		}
	}
	else if (texInfo.textureDimension == rhi::TEXTURE2D)
	{
		if (texInfo.layersCount > 1)
		{
			if (texInfo.resourceFlags == rhi::CUBE_TEXTURE)
			{
				if (texInfo.layersCount > 6)
				{
					createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
				}
				else
				{
					createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
				}
			}
			else
			{
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			}
		}
		else
		{
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		}
	}
	else if (texInfo.textureDimension == rhi::TEXTURE3D)
	{
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
	}
	else
	{
		LOG_ERROR("VulkanRHI::create_texture_view(): Undefined texture dimension")
		return;
	}
	
	//createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.image = vkTexture->_image;
	createInfo.format = format;
	createInfo.subresourceRange.baseMipLevel = viewInfo->baseMipLevel;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = viewInfo->baseLayer;
	createInfo.subresourceRange.layerCount = texInfo.layersCount;
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

	// Using of min max sampler filter
	VkSamplerReductionModeCreateInfo reductionMode{};
	reductionMode.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO;
	switch (sampInfo->filter)
	{
		case rhi::MINIMUM_MIN_MAG_MIP_NEAREST:
		case rhi::MINIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::MINIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::MINIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::MINIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::MINIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::MINIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::MINIMUM_MIN_MAG_MIP_LINEAR:
		case rhi::MINIMUM_ANISOTROPIC:
			reductionMode.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;
			createInfo.pNext = &reductionMode;
			break;
		case rhi::MAXIMUM_MIN_MAG_MIP_NEAREST:
		case rhi::MAXIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::MAXIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::MAXIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::MAXIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::MAXIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::MAXIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::MAXIMUM_MIN_MAG_MIP_LINEAR:
		case rhi::MAXIMUM_ANISOTROPIC:
			reductionMode.reductionMode = VK_SAMPLER_REDUCTION_MODE_MAX;
			createInfo.pNext = &reductionMode;
			break;
	}
	
	VkSampler* vkSampler = new VkSampler();
	VK_CHECK(vkCreateSampler(_vulkanDevice.get_device(), &createInfo, nullptr, vkSampler));
	sampler->handle = vkSampler;
	sampler->sampInfo = *sampInfo;
}

void vulkan::VulkanRHI::create_shader(rhi::Shader* shader, rhi::ShaderInfo* shaderInfo)
{
	if (!shader)
	{
		LOG_ERROR("VulkanRHI::create_shader(): Invalid pointer to rhi::Shader")
		return;
	}
	if (!shaderInfo)
	{
		LOG_ERROR("VulkanRHI::create_shader(): Invalid pointer to rhi::Shader")
		return;
	}
	VulkanShader* vulkanShader = new VulkanShader(_vulkanDevice.get_device());
	vulkanShader->create_shader_module(shaderInfo);
	shader->type = shaderInfo->shaderType;
	shader->handle = vulkanShader;
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
		if (shader.type == rhi::UNDEFINED_SHADER_TYPE)
		{
			LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Shader type is undefined")
			return;
		}
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

	VkPipeline vkPipeline;
	VK_CHECK(vkCreateGraphicsPipelines(_vulkanDevice.get_device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipeline));

	VulkanPipeline* vulkanPipeline = new VulkanPipeline();
	vulkanPipeline->pipeline = vkPipeline;
	vulkanPipeline->pipelineLayout = pipelineLayout;
	pipeline->handle = vulkanPipeline;
}

void vulkan::VulkanRHI::create_compute_pipeline(rhi::Pipeline* pipeline, rhi::ComputePipelineInfo* info)
{
	if (!pipeline)
	{
		LOG_ERROR("VulkanRHI::create_compute_pipeline(): Invalid rhi::Pipeline pointer")
		return;
	}
	if (!info)
	{
		LOG_ERROR("VulkanRHI::create_compute_pipeline(): Invalid ComputePipelieInfo pointer")
		return;
	}

	VkShaderStageFlagBits stageBit = (VkShaderStageFlagBits)get_shader_stage(info->shaderStage.type);
	VulkanShader* vkShader = static_cast<VulkanShader*>(info->shaderStage.handle); 
	VulkanShaderStages vkStage;
	vkStage.add_stage(vkShader, stageBit);
	VkPipelineLayout layout = vkStage.get_pipeline_layout(_vulkanDevice.get_device());

	VkPipelineShaderStageCreateInfo shaderStage{};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.module = vkShader->get_shader_module();
	shaderStage.stage = stageBit;
	shaderStage.pName = "main";
	
	VkComputePipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	createInfo.layout = layout;
	createInfo.stage = shaderStage;

	VkPipeline vkPipeline;
	VK_CHECK(vkCreateComputePipelines(_vulkanDevice.get_device(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &vkPipeline));
	VulkanPipeline* vulkanPipeline = new VulkanPipeline();
	vulkanPipeline->pipeline = vkPipeline;
	vulkanPipeline->pipelineLayout = layout;
	pipeline->handle = vulkanPipeline;
	pipeline->type = rhi::COMPUTE_PIPELINE;
}

void vulkan::VulkanRHI::create_render_pass(rhi::RenderPass* renderPass, rhi::RenderPassInfo* passInfo)
{
	std::vector<VkAttachmentDescription> attachDescriptions;
	std::vector<VkAttachmentReference> colorAttachRefs;
	VkAttachmentReference depthAttachRef;
	bool oneDepth = false;

	if (passInfo->renderTargets.empty())
	{
		LOG_ERROR("VulkanRHI::create_render_pass(): There are no render targets")
		return;
	}
	if (!renderPass)
	{
		LOG_ERROR("VulkanRHI::create_render_pass(): Invalid pointer to rhi::RenderPass")
		return;
	}
	if (!passInfo)
	{
		LOG_ERROR("VulkanRHI::create_render_pass(): Invalid pointer to rhi::RenderPassInfo")
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

	rhi::MultiviewInfo& multiviewInfo = passInfo->multiviewInfo;
	VkRenderPassMultiviewCreateInfoKHR renderPassMultiviewInfo{};
	
	if (multiviewInfo.isEnabled)
	{
		if (!multiviewInfo.viewCount)
		{
			LOG_ERROR("VulkanRHI::create_render_pass(): If multiview is used, view count can't be 0")
			return;
		}
		if (multiviewInfo.viewCount > _vulkanDevice.get_max_multiview_view_count())
		{
			LOG_ERROR("VulkanRHI::create_render_pass(): View count can't be greater than {}", _vulkanDevice.get_max_multiview_view_count())
			return;
		}

		const uint32_t viewMask = (1 << multiviewInfo.viewCount) - 1;
		const uint32_t correlationMask = (1 << multiviewInfo.viewCount) - 1;
		
		renderPassMultiviewInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO_KHR;
		renderPassMultiviewInfo.subpassCount = 1;
		renderPassMultiviewInfo.pViewMasks = &viewMask;
		renderPassMultiviewInfo.correlationMaskCount = 1;
		renderPassMultiviewInfo.pCorrelationMasks = &correlationMask;

		renderPassInfo.pNext = &renderPassMultiviewInfo;
	}

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
