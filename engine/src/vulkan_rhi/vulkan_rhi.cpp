#include "vulkan_rhi.h"
#include "vulkan_common.h"

#include "profiler/logger.h"
#include "file_system/utils.h"
#include <VkBootstrap.h>

#ifndef VMA_IMPLEMENTATION
	#define VMA_IMPLEMENTATION
#endif
#include "vk_mem_alloc.h"

using namespace ad_astris;

void vulkan::VulkanRHI::init(void* window, io::FileSystem* fileSystem)
{
	_fileSystem = fileSystem;
	vkb::Instance vkbInstance = create_instance();
	_instance = vkbInstance.instance;
	_debugMessenger = vkbInstance.debug_messenger;
	_vulkanDevice = std::make_unique<VulkanDevice>(vkbInstance, window);
	create_allocator();
	create_pipeline_cache();
}

// TODO Must test it
void vulkan::VulkanRHI::cleanup()
{
	save_pipeline_cache();
	
	// for (auto& pipeline : _vulkanPipelines)
	// 	pipeline->cleanup();
	//
	// for (auto& shader : _vulkanShaders)
	// 	shader->cleanup();
	//
	// for (auto& renderPass : _vulkanRenderPasses)
	// 	renderPass->cleanup();
	//
	// VkDevice device = _vulkanDevice.get()->get_device();
	// for (auto& sampler : _vulkanSamplers)
	// 	vkDestroySampler(device, *sampler.get(), nullptr);
	//
	// for (auto& imageView : _vulkanImageViews)
	// 	vkDestroyImageView(device, *imageView.get(), nullptr);
	//
	// for (auto& texture : _vulkanTextures)
	// 	texture->destroy_texture();
	//
	// for (auto& buffer : _vulkanBuffers)
	// 	buffer->destroy_buffer(&_allocator);
	//
	// vkDestroyPipelineCache(device, _pipelineCache, nullptr);
	// vmaDestroyAllocator(_allocator);
	// _cmdManager->cleanup();
	// _swapChain->cleanup();
	// _vulkanDevice->cleanup();
	// vkb::destroy_debug_utils_messenger(_instance, _debugMessenger, nullptr);
	// vkDestroyInstance(_instance, nullptr);
}

void vulkan::VulkanRHI::create_swap_chain(rhi::SwapChain* swapChain, rhi::SwapChainInfo* info)
{
	if (!swapChain || !info)
	{
		LOG_ERROR("VulkanRHI::create_swap_chain(): Invalid pointers")
		return;
	}
	_swapChain = std::make_unique<VulkanSwapChain>(info, _vulkanDevice.get());
	_cmdManager = std::make_unique<VulkanCommandManager>(_vulkanDevice.get(), _swapChain.get());
	swapChain->handle = _swapChain.get();
}

void vulkan::VulkanRHI::destroy_swap_chain(rhi::SwapChain* swapChain)
{
	if (!swapChain)
	{
		LOG_ERROR("VulkanRHI::destroy_swap_chain(): Invalid pointer to rhi::SwapChain")
		return;
	}

	_swapChain.reset();
	swapChain->handle = nullptr;
}

void vulkan::VulkanRHI::create_buffer(rhi::Buffer* buffer, rhi::BufferInfo* bufInfo, void* data)
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

	_vulkanBuffers.emplace_back(new VulkanBuffer(&_allocator, bufInfo->size, bufferUsage, memoryUsage));
	VulkanBuffer* vulkanBuffer = _vulkanBuffers.back().get();
	buffer->data = vulkanBuffer;
	buffer->size = bufInfo->size;
	buffer->type = rhi::Resource::ResourceType::BUFFER;
	buffer->bufferInfo = *bufInfo;
	if (data == nullptr)
		return;
	if (data != nullptr && memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY)
	{
		LOG_ERROR("Can't copy data from CPU to buffer if memory usage is VMA_MEMORY_USAGE_GPU_ONLY")
		return;
	}
	vulkanBuffer->copy_from(&_allocator, data, bufInfo->size);
}

void vulkan::VulkanRHI::update_buffer_data(rhi::Buffer* buffer, uint64_t size, void* data)
{
	if (!data || !buffer || !buffer->data || size == 0)
	{
		LOG_ERROR("Can't use update_buffer_data if buffer, data or size is invalid")
		return;
	}
	
	if (buffer->bufferInfo.memoryUsage == rhi::MemoryUsage::GPU)
	{
		LOG_ERROR("Can't copy data from CPU to buffer if memory usage is VMA_MEMORY_USAGE_GPU_ONLY")
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

	if (texInfo->format == rhi::Format::UNDEFINED)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined format")
		return;
	}
	if (texInfo->textureUsage == rhi::ResourceUsage::UNDEFINED)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined texture usage.")
		return;
	}
	if (texInfo->memoryUsage == rhi::MemoryUsage::UNDEFINED)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined memory usage.")
		return;
	}
	if (texInfo->samplesCount == rhi::SampleCount::UNDEFINED)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined sample count.")
		return;
	}
	if (texInfo->textureDimension == rhi::TextureDimension::UNDEFINED)
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
	
	if (has_flag(texInfo->resourceFlags, rhi::ResourceFlags::CUBE_TEXTURE))
		createInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	VkImageUsageFlags imgUsage = get_image_usage(texInfo->textureUsage);
	createInfo.usage = imgUsage;
	createInfo.imageType = get_image_type(texInfo->textureDimension);

	_vulkanTextures.emplace_back(new VulkanTexture(createInfo, &_allocator, memoryUsage));
	VulkanTexture* vkText = _vulkanTextures.back().get();
	if (vkText->get_handle() == VK_NULL_HANDLE)
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
	if ((imgUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	else
		aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

	if (texInfo.textureDimension == rhi::TextureDimension::TEXTURE1D)
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
	else if (texInfo.textureDimension == rhi::TextureDimension::TEXTURE2D)
	{
		if (texInfo.layersCount > 1)
		{
			if (has_flag(texInfo.resourceFlags, rhi::ResourceFlags::CUBE_TEXTURE))
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
	else if (texInfo.textureDimension == rhi::TextureDimension::TEXTURE3D)
	{
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
	}
	else
	{
		LOG_ERROR("VulkanRHI::create_texture_view(): Undefined texture dimension")
		return;
	}
	
	//createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.image = vkTexture->get_handle();
	createInfo.format = format;
	createInfo.subresourceRange.baseMipLevel = viewInfo->baseMipLevel;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = viewInfo->baseLayer;
	createInfo.subresourceRange.layerCount = texInfo.layersCount;
	createInfo.subresourceRange.aspectMask = aspectFlags;

	_vulkanImageViews.emplace_back(new VkImageView());
	VkImageView* view = _vulkanImageViews.back().get();
	VK_CHECK(vkCreateImageView(_vulkanDevice->get_device(), &createInfo, nullptr, view));
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
	if (sampInfo->addressMode == rhi::AddressMode::UNDEFINED)
	{
		LOG_ERROR("VulkanRHI::create_sampler(): Undefined address mode. Failed to create VkSampler")
		return;
	}
	if (sampInfo->filter == rhi::Filter::UNDEFINED)
	{
		LOG_ERROR("VulkanRHI::create_sampler(): Undefined filter. Failed to create VkSampler")
		return;
	}
	if (sampInfo->borderColor == rhi::BorderColor::UNDEFINED)
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
	if (sampInfo->borderColor != rhi::BorderColor::UNDEFINED)
		createInfo.borderColor = get_border_color(sampInfo->borderColor);

	// Using of min max sampler filter
	VkSamplerReductionModeCreateInfo reductionMode{};
	reductionMode.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO;
	switch (sampInfo->filter)
	{
		case rhi::Filter::MINIMUM_MIN_MAG_MIP_NEAREST:
		case rhi::Filter::MINIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::Filter::MINIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::Filter::MINIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::Filter::MINIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::Filter::MINIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::Filter::MINIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::Filter::MINIMUM_MIN_MAG_MIP_LINEAR:
		case rhi::Filter::MINIMUM_ANISOTROPIC:
			reductionMode.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;
			createInfo.pNext = &reductionMode;
			break;
		case rhi::Filter::MAXIMUM_MIN_MAG_MIP_NEAREST:
		case rhi::Filter::MAXIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::Filter::MAXIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::Filter::MAXIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::Filter::MAXIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::Filter::MAXIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::Filter::MAXIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::Filter::MAXIMUM_MIN_MAG_MIP_LINEAR:
		case rhi::Filter::MAXIMUM_ANISOTROPIC:
			reductionMode.reductionMode = VK_SAMPLER_REDUCTION_MODE_MAX;
			createInfo.pNext = &reductionMode;
			break;
	}

	_vulkanSamplers.emplace_back(new VkSampler());
	VkSampler* vkSampler = _vulkanSamplers.back().get();
	VK_CHECK(vkCreateSampler(_vulkanDevice->get_device(), &createInfo, nullptr, vkSampler));
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

	_vulkanShaders.emplace_back(new VulkanShader(_vulkanDevice->get_device()));
	VulkanShader* vulkanShader = _vulkanShaders.back().get();
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
	_vulkanPipelines.emplace_back(new VulkanPipeline(_vulkanDevice.get(), info, _pipelineCache));
	pipeline->type = rhi::PipelineType::GRAPHICS;
	pipeline->handle = _vulkanPipelines.back().get();
}

void vulkan::VulkanRHI::create_compute_pipeline(rhi::Pipeline* pipeline, rhi::ComputePipelineInfo* info)
{
	if (!pipeline || !info)
	{
		LOG_ERROR("VulkanRHI::create_compute_pipeline(): Invalid pointers")
		return;
	}
	_vulkanPipelines.emplace_back(new VulkanPipeline(_vulkanDevice.get(), info, _pipelineCache));
	pipeline->type = rhi::PipelineType::COMPUTE;
	pipeline->handle = _vulkanPipelines.back().get();
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

	_vulkanRenderPasses.emplace_back(new VulkanRenderPass(_vulkanDevice.get(), passInfo));
	renderPass->handle = _vulkanRenderPasses.back().get();
}

// TODO Queue type for command buffer
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

void vulkan::VulkanRHI::copy_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* srcBuffer, rhi::Buffer* dstBuffer, uint32_t size, uint32_t srcOffset, uint32_t dstOffset)
{
	if (!cmd || !srcBuffer || !dstBuffer)
	{
		LOG_INFO("VulkanRHI::copy_buffer(): Invalid pointers")
		return;
	}
	if (!has_flag(srcBuffer->bufferInfo.bufferUsage, rhi::ResourceUsage::TRANSFER_SRC))
	{
		LOG_INFO("VulkanRHI::copy_buffer(): Source buffer doesn't have TRANSFER_SRC usage")
		return;
	}
	if (!has_flag(dstBuffer->bufferInfo.bufferUsage, rhi::ResourceUsage::TRANSFER_DST))
	{
		LOG_INFO("VulkanRHI::copy_buffer(): Destination buffer doesn't have TRANSFER_DST usage")
		return;
	}
	// TODO Maybe here is a bug with command buffer, I have to test this method
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	//VkCommandBuffer vkCmd = *static_cast<VkCommandBuffer*>(cmd->handle);
	VulkanBuffer* vkSrcBuffer = get_vk_obj(srcBuffer);
	VulkanBuffer* vkDstBuffer = get_vk_obj(dstBuffer);

	VkBufferCopy copy;
	copy.srcOffset = srcOffset;
	copy.dstOffset = dstOffset;
	if (!size)
	{
		copy.size = srcBuffer->size;
	}
	else
	{
		copy.size = size;
	}

	vkCmdCopyBuffer(vkCmd->get_handle(), *vkSrcBuffer->get_handle(), *vkDstBuffer->get_handle(), 1, &copy);
}

void vulkan::VulkanRHI::blit_texture(rhi::CommandBuffer* cmd, rhi::Texture* srcTexture, rhi::Texture* dstTexture, std::array<int32_t, 3>& srcOffset, std::array<int32_t, 3>& dstOffset, uint32_t srcMipLevel, uint32_t dstMipLevel, uint32_t srcBaseLayer, uint32_t dstBaseLayer)
{
	if (!cmd || !srcTexture || !dstTexture)
	{
		LOG_INFO("VulkanRHI::blit_texture(): Invalid pointers")
		return;
	}
	if (!has_flag(srcTexture->textureInfo.textureUsage, rhi::ResourceUsage::TRANSFER_SRC))
	{
		LOG_INFO("VulkanRHI::blit_texture(): Source buffer doesn't have TRANSFER_SRC usage")
		return;
	}
	if (!has_flag(dstTexture->textureInfo.textureUsage, rhi::ResourceUsage::TRANSFER_DST))
	{
		LOG_INFO("VulkanRHI::blit_texture(): Destination buffer doesn't have TRANSFER_DST usage")
		return;
	}

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanTexture* vkSrcTexture = get_vk_obj(srcTexture);
	VulkanTexture* vkDstTexture = get_vk_obj(dstTexture);
	rhi::TextureInfo& srcInfo = srcTexture->textureInfo;
	rhi::TextureInfo& dstInfo = dstTexture->textureInfo;

	VkImageBlit imageBlit{};
	imageBlit.srcOffsets[1].x = srcOffset[0];
	imageBlit.srcOffsets[1].y = srcOffset[1];
	imageBlit.srcOffsets[1].z = srcOffset[2];
	imageBlit.dstOffsets[1].x = dstOffset[0];
	imageBlit.dstOffsets[1].y = dstOffset[1];
	imageBlit.dstOffsets[1].x = dstOffset[2];
	
	imageBlit.srcSubresource.aspectMask = get_image_aspect(srcInfo.textureUsage);
	imageBlit.srcSubresource.layerCount = srcInfo.layersCount;
	imageBlit.srcSubresource.baseArrayLayer = srcBaseLayer;
	imageBlit.srcSubresource.mipLevel = srcMipLevel;
	imageBlit.dstSubresource.aspectMask = get_image_aspect(dstInfo.textureUsage);
	imageBlit.dstSubresource.layerCount = dstInfo.layersCount;
	imageBlit.dstSubresource.baseArrayLayer = dstBaseLayer;
	imageBlit.dstSubresource.mipLevel = dstMipLevel;

	vkCmdBlitImage(
		vkCmd->get_handle(),
		vkSrcTexture->get_handle(),
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		vkDstTexture->get_handle(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&imageBlit,
		VK_FILTER_LINEAR);
}

void vulkan::VulkanRHI::copy_buffer_to_texture(rhi::CommandBuffer* cmd, rhi::Buffer* srcBuffer, rhi::Texture* dstTexture, rhi::ResourceUsage textureUsage)
{
	if (!cmd || !srcBuffer || !dstTexture)
	{
		LOG_INFO("VulkanRHI::copy_buffer_to_texture(): Invalid pointers")
		return;
	}
	if (!has_flag(srcBuffer->bufferInfo.bufferUsage, rhi::ResourceUsage::TRANSFER_SRC))
	{
		LOG_INFO("VulkanRHI::copy_buffer_to_texture(): Source buffer doesn't have TRANSFER_SRC usage")
		return;
	}
	if (!has_flag(dstTexture->textureInfo.textureUsage, rhi::ResourceUsage::TRANSFER_DST))
	{
		LOG_INFO("VulkanRHI::copy_buffer_to_texture(): Destination buffer doesn't have TRANSFER_DST usage")
		return;
	}

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanBuffer* vkBuffer = get_vk_obj(srcBuffer);
	VulkanTexture* vkTexture = get_vk_obj(dstTexture);
	rhi::TextureInfo& texInfo = dstTexture->textureInfo;

	VkImageMemoryBarrier imageBarrierToTransfer{};
	imageBarrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrierToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageBarrierToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrierToTransfer.srcAccessMask = 0;
	imageBarrierToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageBarrierToTransfer.image = vkTexture->get_handle();
	imageBarrierToTransfer.subresourceRange = {
		get_image_aspect(texInfo.textureUsage),
		0,
		texInfo.mipLevels,
		0,
		texInfo.layersCount };

	vkCmdPipelineBarrier(
		vkCmd->get_handle(),
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, nullptr, 0, nullptr,
		1, &imageBarrierToTransfer);
	
	VkBufferImageCopy copyRegion{};
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;
	copyRegion.imageExtent = vkTexture->get_extent();
	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = texInfo.layersCount;
	vkCmdCopyBufferToImage(vkCmd->get_handle(), *vkBuffer->get_handle(), vkTexture->get_handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	VkImageMemoryBarrier imageBarrierToReadable = imageBarrierToTransfer;
	imageBarrierToReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrierToReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageBarrierToReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageBarrierToTransfer.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		vkCmd->get_handle(),
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 0, nullptr, 0, nullptr,
		1, &imageBarrierToReadable);
}

void vulkan::VulkanRHI::set_viewport(rhi::CommandBuffer* cmd, float width, float height)
{
	if (!cmd)
	{
		LOG_ERROR("VulkanRHI::set_viewport(): Invalid pointer")
	}
	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.width = width;
	viewport.height = height;

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	vkCmdSetViewport(vkCmd->get_handle(), 0, 1, &viewport);
}

void vulkan::VulkanRHI::set_scissor(rhi::CommandBuffer* cmd, uint32_t width, uint32_t height, int32_t offsetX, int32_t offsetY)
{
	if (!cmd)
	{
		LOG_ERROR("VulkanRHI::set_scissor(): Invalid pointer")
		return;
	}
	VkExtent2D extent = { width, height };
	VkRect2D scissor;
	scissor.extent = extent;
	scissor.offset = { offsetX, offsetY };
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	vkCmdSetScissor(vkCmd->get_handle(), 0, 1, &scissor);
}

void vulkan::VulkanRHI::bind_vertex_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer)
{
	if (!cmd || !buffer)
	{
		LOG_ERROR("VulkanRHI::bind_vertex_buffer(): Invalid pointers")
		return;
	}
	if (has_flag(buffer->bufferInfo.bufferUsage, rhi::ResourceUsage::VERTEX_BUFFER))
	{
		LOG_ERROR("VulkanRHI::bind_vertex_buffer(): Buffer wasn't created with VERTEX_BUFFER usage")
		return;
	}
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanBuffer* vkBuffer = get_vk_obj(buffer);

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
	if (has_flag(buffer->bufferInfo.bufferUsage, rhi::ResourceUsage::INDEX_BUFFER))
	{
		LOG_ERROR("VulkanRHI::bind_index_buffer(): Buffer wasn't created with INDEX_BUFFER usage")
		return;
	}
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanBuffer* vkBuffer = get_vk_obj(buffer);

	VkDeviceSize offset = 0;
	vkCmdBindIndexBuffer(vkCmd->get_handle(), *vkBuffer->get_handle(), offset, VK_INDEX_TYPE_UINT32);
}

void vulkan::VulkanRHI::bind_pipeline(rhi::CommandBuffer* cmd, rhi::Pipeline* pipeline)
{
	if (!cmd || !pipeline)
	{
		LOG_ERROR("VulkanRHI::bind_pipeline(): Invalid pointers")
		return;
	}
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanPipeline* vkPipeline = get_vk_obj(pipeline);
	vkCmdBindPipeline(vkCmd->get_handle(), get_pipeline_bind_point(vkPipeline->get_type()), vkPipeline->get_handle());
}

void vulkan::VulkanRHI::begin_render_pass(rhi::CommandBuffer* cmd, rhi::RenderPass* renderPass)
{
	if (!cmd || !renderPass)
	{
		LOG_ERROR("VulkanRHI::bind_render_pass(): Invalid pointers")
		return;
	}

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanRenderPass* pass = get_vk_obj(renderPass);

	VkRenderPassBeginInfo beginInfo = pass->get_begin_info();

	vkCmdBeginRenderPass(vkCmd->get_handle(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void vulkan::VulkanRHI::end_render_pass(rhi::CommandBuffer* cmd)
{
	if (!cmd)
	{
		LOG_ERROR("VulkanRHI::end_render_pass(): Invalid pointer")
		return;
	}
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	vkCmdEndRenderPass(vkCmd->get_handle());
}

void vulkan::VulkanRHI::draw(rhi::CommandBuffer* cmd, uint64_t vertexCount)
{
	if (!cmd)
	{
		LOG_ERROR("VulkanRHI::draw(): Invalid pointer")
		return;
	}
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	vkCmdDraw(vkCmd->get_handle(), vertexCount, 1, 0, 0);
}

void vulkan::VulkanRHI::draw_indexed(
	rhi::CommandBuffer* cmd,
	uint32_t indexCount,
	uint32_t instanceCount,
	uint32_t firstIndex,
	int32_t vertexOffset,
	uint32_t firstInstance)
{
	if (!cmd)
	{
		LOG_ERROR("VulkanRHI::draw_indexed(): Invalid pointer")
		return;
	}
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	vkCmdDrawIndexed(vkCmd->get_handle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void vulkan::VulkanRHI::draw_indirect(rhi::CommandBuffer* cmd, rhi::Buffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride)
{
	if (!cmd || !buffer)
	{
		LOG_ERROR("VulkanRHI::draw_indirect(): Invalid pointers")
		return;
	}
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanBuffer* vkBuffer = get_vk_obj(buffer);
	vkCmdDrawIndirect(vkCmd->get_handle(), *vkBuffer->get_handle(), offset, drawCount, stride);
}

void vulkan::VulkanRHI::draw_indexed_indirect(rhi::CommandBuffer* cmd, rhi::Buffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride)
{
	if (!cmd || !buffer)
	{
		LOG_ERROR("VulkanRHI::draw_indirect(): Invalid pointers")
		return;
	}
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanBuffer* vkBuffer = get_vk_obj(buffer);
	vkCmdDrawIndexedIndirect(vkCmd->get_handle(), *vkBuffer->get_handle(), offset, drawCount, stride);
}

void vulkan::VulkanRHI::dispatch(rhi::CommandBuffer* cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	if (!cmd)
	{
		LOG_ERROR("VulkanRHI::dispatch(): Invalid pointer")
		return;
	}
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	vkCmdDispatch(vkCmd->get_handle(), groupCountX, groupCountY, groupCountZ);
}

void vulkan::VulkanRHI::fill_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer, uint32_t dstOffset, uint32_t size, uint32_t data)
{
	if (!cmd || !buffer)
	{
		LOG_ERROR("VulkanRHI::fill_buffer(): Invalid pointers")
		return;
	}
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanBuffer* vkBuffer = get_vk_obj(buffer);
	vkCmdFillBuffer(vkCmd->get_handle(), *vkBuffer->get_handle(), dstOffset, size, data);
}

void vulkan::VulkanRHI::add_pipeline_barriers(rhi::CommandBuffer* cmd, std::vector<rhi::PipelineBarrier>& barriers)
{
	std::vector<VkMemoryBarrier> memoryBarriers;
	std::vector<VkBufferMemoryBarrier> bufferBarriers;
	std::vector<VkImageMemoryBarrier> imageBarriers;
	for (auto& barrier : barriers)
	{
		switch (barrier.type)
		{
			case rhi::PipelineBarrier::BarrierType::MEMORY:
			{
				VkMemoryBarrier memoryBarrier{};
				memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
				memoryBarrier.srcAccessMask = get_access(barrier.memoryBarrier.srcLayout);
				memoryBarrier.dstAccessMask = get_access(barrier.memoryBarrier.dstLayout);
				memoryBarriers.push_back(memoryBarrier);
				break;
			}
			case rhi::PipelineBarrier::BarrierType::BUFFER:
			{
				VkBufferMemoryBarrier bufferBarrier{};
				bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				bufferBarrier.srcAccessMask = get_access(barrier.bufferBarrier.srcLayout);
				bufferBarrier.dstAccessMask = get_access(barrier.bufferBarrier.dstLayout);
				bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferBarriers.push_back(bufferBarrier);
				break;
			}
			case rhi::PipelineBarrier::BarrierType::TEXTURE:
			{
				VkImageMemoryBarrier imageBarrier{};
				imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imageBarrier.srcAccessMask = get_access(barrier.textureBarrier.srcLayout);
				imageBarrier.dstAccessMask = get_access(barrier.textureBarrier.dstLayout);
				imageBarrier.oldLayout = get_image_layout(barrier.textureBarrier.srcLayout);
				imageBarrier.newLayout = get_image_layout(barrier.textureBarrier.dstLayout);
				rhi::PipelineBarrier::TextureBarrier texBarrier = barrier.textureBarrier;
				VulkanTexture* vkTexture = get_vk_obj(texBarrier.texture);
				imageBarrier.image = vkTexture->get_handle();
				VkImageSubresourceRange range;
				if (has_flag(texBarrier.texture->textureInfo.textureUsage, rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT))
				{
					range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				}
				else
				{
					range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				}
				range.layerCount = VK_REMAINING_ARRAY_LAYERS;
				range.baseArrayLayer = texBarrier.baseLayer;
				range.levelCount = VK_REMAINING_MIP_LEVELS;
				range.baseMipLevel = texBarrier.baseMipLevel;
				imageBarrier.subresourceRange = range;
				imageBarriers.push_back(imageBarrier);
				break;
			}
		}
	}

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	vkCmdPipelineBarrier(
		vkCmd->get_handle(),
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		0,
		memoryBarriers.size(),
		memoryBarriers.data(),
		bufferBarriers.size(),
		bufferBarriers.data(),
		imageBarriers.size(),
		imageBarriers.data());
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
	allocatorInfo.physicalDevice = _vulkanDevice->get_physical_device();
	allocatorInfo.device = _vulkanDevice->get_device();
	allocatorInfo.instance = _instance;
	vmaCreateAllocator(&allocatorInfo, &_allocator);
	LOG_INFO("End creating allocator")
}

void vulkan::VulkanRHI::create_pipeline_cache()
{
	size_t size = 0;
	void* data = nullptr;
	
	if (io::Utils::exists(_fileSystem, "configs/pipeline_cache.bin"))
		data = _fileSystem->map_to_read("configs/pipeline_cache.bin", size);

	VkPipelineCacheCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	createInfo.initialDataSize = size;
	createInfo.pInitialData = data;

	VK_CHECK(vkCreatePipelineCache(_vulkanDevice.get()->get_device(), &createInfo, nullptr, &_pipelineCache));

	if (data)
		_fileSystem->unmap_after_reading(data);
}

void vulkan::VulkanRHI::save_pipeline_cache()
{
	VkDevice device = _vulkanDevice.get()->get_device();
	size_t cacheSize;
	vkGetPipelineCacheData(device, _pipelineCache, &cacheSize, nullptr);
	std::vector<uint8_t> cacheData(cacheSize);
	vkGetPipelineCacheData(device, _pipelineCache, &cacheSize, cacheData.data());

	io::Stream* stream = _fileSystem->open("configs/pipeline_cache.bin", "wb");
	stream->write(cacheData.data(), sizeof(uint8_t), cacheSize);
	_fileSystem->close(stream);
}
