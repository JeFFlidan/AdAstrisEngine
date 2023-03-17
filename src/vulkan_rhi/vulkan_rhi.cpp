#include "vulkan_rhi.h"

#include <valarray>

#include "vulkan_common.h"
#include "vulkan_buffer.h"
#include "vulkan_texture.h"
#include "vulkan_shader.h"
#include "vulkan_pipeline.h"
#include "vulkan_render_pass.h"
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
	delete _swapChain;
	delete _cmdManager;
}

void vulkan::VulkanRHI::create_swap_chain(rhi::SwapChain* swapChain, rhi::SwapChainInfo* info)
{
	if (!swapChain || !info)
	{
		LOG_ERROR("VulkanRHI::create_swap_chain(): Invalid pointers")
		return;
	}
	_swapChain = new VulkanSwapChain(info, &_vulkanDevice);
	_cmdManager = new VulkanCommandManager(&_vulkanDevice, _swapChain);
	swapChain->handle = _swapChain;
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
	swapChain->handle = nullptr;
	_swapChain = nullptr;
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
	if (!pipeline || !info)
	{
		LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Invalid pointers")
		return;
	}
	pipeline->type = rhi::PipelineType::GRAPHICS_PIPELINE;
	pipeline->handle = new VulkanPipeline(&_vulkanDevice, info);
}

void vulkan::VulkanRHI::create_compute_pipeline(rhi::Pipeline* pipeline, rhi::ComputePipelineInfo* info)
{
	if (!pipeline || !info)
	{
		LOG_ERROR("VulkanRHI::create_compute_pipeline(): Invalid pointers")
		return;
	}
	pipeline->type = rhi::COMPUTE_PIPELINE;
	pipeline->handle = new VulkanPipeline(&_vulkanDevice, info);
}

void vulkan::VulkanRHI::create_render_pass(rhi::RenderPass* renderPass, rhi::RenderPassInfo* passInfo)
{
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

	renderPass->handle = new VulkanRenderPass(&_vulkanDevice, passInfo);
}

void vulkan::VulkanRHI::begin_command_buffer(rhi::CommandBuffer* cmd, rhi::QueueType)
{
	cmd->handle = _cmdManager->get_command_buffer();
}

void vulkan::VulkanRHI::wait_command_buffer(rhi::CommandBuffer* cmd, rhi::CommandBuffer* waitForCmd)
{
	VulkanCommandBuffer* cmd1 = static_cast<VulkanCommandBuffer*>(cmd->handle);
	VulkanCommandBuffer* cmd2 = static_cast<VulkanCommandBuffer*>(waitForCmd->handle);
	_cmdManager->wait_for_cmd_buffer(cmd1, cmd2);
}

void vulkan::VulkanRHI::submit(rhi::QueueType queueType)
{
	_cmdManager->submit(queueType);
}

void vulkan::VulkanRHI::bind_vertex_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer)
{
	if (!cmd || !buffer)
	{
		LOG_ERROR("VulkanRHI::bind_vertex_buffer(): Invalid pointers")
		return;
	}
	if (buffer->bufferInfo.bufferUsage != rhi::VERTEX_BUFFER)
	{
		LOG_ERROR("VulkanRHI::bind_vertex_buffer(): Buffer wasn't created with VERTEX_BUFFER usage")
		return;
	}
	VulkanCommandBuffer* vkCmd = static_cast<VulkanCommandBuffer*>(cmd->handle);
	VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer->data);

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(vkCmd->get_handle(), 0, 1, vkBuffer->get_handle(), &offset);
}

void vulkan::VulkanRHI::bind_index_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer)
{
	if (!cmd || !buffer)
	{
		LOG_ERROR("VulkanRHI::bind_index_buffer(): Invalid pointers")
		return;
	}
	if (buffer->bufferInfo.bufferUsage != rhi::INDEX_BUFFER)
	{
		LOG_ERROR("VulkanRHI::bind_index_buffer(): Buffer wasn't created with INDEX_BUFFER usage")
		return;
	}
	VulkanCommandBuffer* vkCmd = static_cast<VulkanCommandBuffer*>(cmd->handle);
	VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer->data);

	VkDeviceSize offset = 0;
	vkCmdBindIndexBuffer(vkCmd->get_handle(), *vkBuffer->get_handle(), offset, VK_INDEX_TYPE_UINT32);
}

void vulkan::VulkanRHI::begin_render_pass(rhi::CommandBuffer* cmd, rhi::RenderPass* renderPass)
{
	if (!cmd || !renderPass)
	{
		LOG_ERROR("VulkanRHI::bind_render_pass(): Invalid pointers")
		return;
	}

	VulkanCommandBuffer* vkCmd = static_cast<VulkanCommandBuffer*>(cmd->handle);
	VulkanRenderPass* pass = static_cast<VulkanRenderPass*>(renderPass->handle);

	VkRenderPassBeginInfo beginInfo = pass->get_begin_info();

	vkCmdBeginRenderPass(vkCmd->get_handle(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void vulkan::VulkanRHI::end_render_pass(rhi::CommandBuffer* cmd)
{
	
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
