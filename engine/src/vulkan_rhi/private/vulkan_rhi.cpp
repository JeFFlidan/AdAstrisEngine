#include "vulkan_rhi.h"
#include "vulkan_queue.h"
#include "vulkan_common.h"

#include "rhi/utils.h"
#include "profiler/logger.h"
#include "file_system/utils.h"

#ifndef VMA_IMPLEMENTATION
	#define VMA_IMPLEMENTATION
#endif
#include <vma/vk_mem_alloc.h>

#ifndef VOLK_IMPLEMENTATION
	#define VOLK_IMPLEMENTATION
#endif
#include <volk/volk.h>

#include <algorithm>

using namespace ad_astris;
using namespace vulkan;

constexpr uint32_t MAX_VIEWPORT_COUNT = 16;
constexpr uint32_t MAX_SCISSOR_COUNT = 16;

void VulkanRHI::init(rhi::RHIInitContext& initContext)
{
	assert(initContext.window != nullptr);
	assert(initContext.swapChainInfo != nullptr);

	_instance = std::make_unique<VulkanInstance>(initContext.validationMode);
	_device = std::make_unique<VulkanDevice>(_instance.get(), initContext.gpuPreference);
	
	_mainSwapChain = std::make_unique<VulkanSwapChain>(_device.get(), initContext.swapChainInfo, initContext.window);
	_cmdManager = std::make_unique<VulkanCommandManager>(_device.get(), _mainSwapChain.get());
	
	_descriptorManager = std::make_unique<VulkanDescriptorManager>(_device.get(), _mainSwapChain->get_buffers_count());
	_pipelineLayoutCache = std::make_unique<VulkanPipelineLayoutCache>(_device.get(), _descriptorManager.get());
	_pipelineCache.load_pipeline_cache(_device.get());

	print_gpu_info();
	
	auto& physicalProperties = _device->get_physical_device_properties();
	_gpuProperties.timestampFrequency = uint64_t(1.0 / (double)physicalProperties.properties.limits.timestampPeriod * 1000 * 1000 * 1000);
	_gpuProperties.bufferCount = _mainSwapChain->get_buffers_count();
	_device->fill_gpu_properties(_gpuProperties);
}

// TODO Must test it
void VulkanRHI::cleanup()
{
	_pipelineCache.save_pipeline_cache(_device.get());
	_mainSwapChain->destroy(_device.get());
	_vkObjectPool.cleanup(_device.get());
	_pipelineCache.destroy(_device.get());
	_pipelineLayoutCache->cleanup();
	_descriptorManager->cleanup();
	_cmdManager->cleanup();
	_device->cleanup();
	_instance->cleanup();
}

void VulkanRHI::create_swap_chain(rhi::SwapChain* swapChain, rhi::SwapChainInfo* info, acore::IWindow* window)
{
	assert(swapChain && info);
	
	swapChain->handle = _vkObjectPool.allocate<VulkanSwapChain>(_device.get(), info, window);
	swapChain->info = *info;
}

void VulkanRHI::destroy_swap_chain(rhi::SwapChain* swapChain)
{
	assert(swapChain);
	
	_vkObjectPool.free(static_cast<VulkanSwapChain*>(swapChain->handle), _device.get());
	swapChain->handle = nullptr;
}

void VulkanRHI::get_swap_chain_texture_views(std::vector<rhi::TextureView>& textureViews)
{
	// for (auto& textureView : _swapChain->get_texture_views())
	// 	textureViews.push_back(textureView);
}

void VulkanRHI::reset_cmd_buffers(uint32_t currentFrameIndex)
{
	_currentImageIndex = currentFrameIndex;
	_cmdManager->reset_cmd_buffers(currentFrameIndex);
}

void VulkanRHI::create_buffer(rhi::Buffer* buffer, rhi::BufferInfo* bufInfo, void* data)
{
	assert(buffer && bufInfo);
	
	buffer->bufferInfo = *bufInfo;
	create_buffer(buffer, data);
}

void VulkanRHI::create_buffer(rhi::Buffer* buffer, void* data)
{
	assert(buffer);
	
	auto vkBuffer = _vkObjectPool.allocate<VulkanBuffer>(_device.get(), &buffer->bufferInfo);

	if (vkBuffer->get_handle() == VK_NULL_HANDLE)
		LOG_FATAL("VulkanRHI::create_buffer(): Failed to allocate buffer")
	
	buffer->handle = vkBuffer;
	buffer->mappedData = vkBuffer->get_allocation()->GetMappedData();
	buffer->mappedDataSize = vkBuffer->get_allocation()->GetSize();
	buffer->type = rhi::Resource::ResourceType::BUFFER;

	if (has_flag(buffer->bufferInfo.bufferUsage, rhi::ResourceUsage::STORAGE_BUFFER))
	{
		VulkanBufferView bufferView(buffer, VK_WHOLE_SIZE, 0);
		rhi::BufferView rhiBufferView{};
		_descriptorManager->allocate_bindless_descriptor(&bufferView, &rhiBufferView);
		vkBuffer->set_descriptor_index(bufferView.get_descriptor_index());
	}
	
	if (data == nullptr)
		return;
	if (buffer->bufferInfo.memoryUsage == rhi::MemoryUsage::GPU)
		LOG_FATAL("VulkanRHI::create_buffer(): Can't copy data from CPU to buffer if memory usage is VMA_MEMORY_USAGE_GPU_ONLY")

	memcpy(buffer->mappedData, data, buffer->bufferInfo.size);
}

void VulkanRHI::destroy_buffer(rhi::Buffer* buffer)
{
	VulkanBuffer* vulkanBuffer = get_vk_obj(buffer);
	_vkObjectPool.free(vulkanBuffer, _device.get());
	
	buffer->handle = nullptr;
}

void VulkanRHI::update_buffer_data(rhi::Buffer* buffer, uint64_t size, void* data)
{
	assert(data && buffer->handle && size);
	
	if (buffer->bufferInfo.memoryUsage == rhi::MemoryUsage::GPU)
		LOG_FATAL("VulkanRHI::update_buffer_data(): Can't copy data from CPU to buffer if memory usage is VMA_MEMORY_USAGE_GPU_ONLY")

	VulkanBuffer* vulkanBuffer = static_cast<VulkanBuffer*>(buffer->handle);
	memcpy(buffer->mappedData, data, size);
}

void VulkanRHI::create_texture(rhi::Texture* texture, rhi::TextureInfo* texInfo)
{
	assert(texture && texInfo);

	texture->textureInfo = *texInfo;
	create_texture(texture);
}

void VulkanRHI::create_texture(rhi::Texture* texture)
{
	assert(texture);

	VkImageCreateInfo createInfo{};
	auto vkTexture = _vkObjectPool.allocate<VulkanTexture>(_device.get(), &texture->textureInfo, createInfo);
	
	texture->handle = vkTexture;
	texture->mappedData = vkTexture->get_allocation()->GetMappedData();
	texture->mappedDataSize = vkTexture->get_allocation()->GetSize();
	texture->type = rhi::Resource::ResourceType::TEXTURE;

	_attachmentManager.add_attachment_texture(vkTexture, createInfo);
}

void VulkanRHI::create_texture_view(rhi::TextureView* textureView, rhi::TextureViewInfo* viewInfo, rhi::Texture* texture)
{
	assert(textureView && viewInfo && texture);

	textureView->viewInfo = *viewInfo;
	create_texture_view(textureView, texture);
}

void VulkanRHI::create_texture_view(rhi::TextureView* textureView, rhi::Texture* texture)
{
	assert(textureView && texture);

	rhi::TextureViewInfo* viewInfo = &textureView->viewInfo;
	
	VkImageViewCreateInfo createInfo{};
	auto vkTextureView = _vkObjectPool.allocate<VulkanTextureView>(_device.get(), viewInfo, texture, createInfo);

	if (!vkTextureView)
		LOG_FATAL("VulkanRHI::create_texture_view(): Failed to create texture view")
	
	textureView->texture = texture;
	textureView->handle = vkTextureView;

	rhi::TextureInfo texInfo = texture->textureInfo;
	VkImageUsageFlags imgUsage = get_image_usage(texInfo.textureUsage);

	_descriptorManager->allocate_bindless_descriptor(vkTextureView, textureView);

	VulkanTexture* vkTexture = get_vk_obj(texture);
	_attachmentManager.add_attachment_texture_view(vkTextureView, vkTexture, imgUsage, createInfo);
}

void VulkanRHI::create_buffer_view(rhi::BufferView* bufferView, rhi::BufferViewInfo* info, rhi::Buffer* buffer)
{
	assert(bufferView && info && buffer);
	bufferView->info = *info;
	create_buffer_view(bufferView, buffer);
}

void VulkanRHI::create_buffer_view(rhi::BufferView* bufferView, rhi::Buffer* buffer)
{
	assert(bufferView && buffer);

	rhi::BufferViewInfo* viewInfo = &bufferView->info;
	auto vkBufferView = _vkObjectPool.allocate<VulkanBufferView>(_device.get(), viewInfo, buffer);

	bufferView->handle = vkBufferView;
	bufferView->buffer = buffer;

	_descriptorManager->allocate_bindless_descriptor(vkBufferView, bufferView);
}

void VulkanRHI::create_sampler(rhi::Sampler* sampler, rhi::SamplerInfo* sampInfo)
{
	assert(sampler && sampInfo);
	
	if (sampInfo->addressMode == rhi::AddressMode::UNDEFINED)
		LOG_FATAL("VulkanRHI::create_sampler(): Undefined address mode. Failed to create VkSampler")
	
	if (sampInfo->filter == rhi::Filter::UNDEFINED)
		LOG_FATAL("VulkanRHI::create_sampler(): Undefined filter. Failed to create VkSampler")
	
	auto vkSampler = _vkObjectPool.allocate<VulkanSampler>(_device.get(), sampInfo);
	sampler->handle = vkSampler;
	sampler->sampInfo = *sampInfo;

	_descriptorManager->allocate_bindless_descriptor(vkSampler);
}

void VulkanRHI::create_shader(rhi::Shader* shader, rhi::ShaderInfo* shaderInfo)
{
	assert(shader && shaderInfo);

	auto vkShader = _vkObjectPool.allocate<VulkanShader>(_device->get_device());
	vkShader->create_shader_module(shaderInfo);
	shader->type = shaderInfo->shaderType;
	shader->handle = vkShader;
}

void VulkanRHI::create_graphics_pipeline(rhi::Pipeline* pipeline, rhi::GraphicsPipelineInfo* info)
{
	assert(pipeline && info);
	
	auto vulkanPipeline = _vkObjectPool.allocate<VulkanPipeline>(_device.get(), info, _pipelineCache.get_handle(), _pipelineLayoutCache.get());
	
	pipeline->type = rhi::PipelineType::GRAPHICS;
	pipeline->handle = vulkanPipeline;
}

void VulkanRHI::create_compute_pipeline(rhi::Pipeline* pipeline, rhi::ComputePipelineInfo* info)
{
	assert(pipeline && info);
	
	auto vulkanPipeline = _vkObjectPool.allocate<VulkanPipeline>(_device.get(), info, _pipelineCache.get_handle(), _pipelineLayoutCache.get());
	
	pipeline->type = rhi::PipelineType::COMPUTE;
	pipeline->handle = vulkanPipeline;
}

void VulkanRHI::create_render_pass(rhi::RenderPass* renderPass, rhi::RenderPassInfo* passInfo)
{
	assert(renderPass && passInfo);
	
	if (passInfo->renderBuffers.empty())
		LOG_FATAL("VulkanRHI::create_render_pass(): There are no render targets")

	auto vkRenderPass = _vkObjectPool.allocate<VulkanRenderPass>(_device.get(), passInfo);
	renderPass->handle = vkRenderPass;
}

uint32_t VulkanRHI::get_descriptor_index(rhi::Buffer* buffer)
{
	return get_vk_obj(buffer)->get_descriptor_index();
}

uint32_t VulkanRHI::get_descriptor_index(rhi::TextureView* textureView)
{
	return get_vk_obj(textureView)->get_descriptor_index();
}

uint32_t VulkanRHI::get_descriptor_index(rhi::BufferView* bufferView)
{
	return get_vk_obj(bufferView)->get_descriptor_index();
}

uint32_t VulkanRHI::get_descriptor_index(rhi::Sampler* sampler)
{
	return get_vk_obj(sampler)->get_descriptor_index();
}

void VulkanRHI::bind_uniform_buffer(rhi::Buffer* buffer, uint32_t slot, uint32_t size, uint32_t offset)
{
	assert(buffer);
	
	rhi::BufferInfo& bufferInfo = buffer->bufferInfo;
	if (!has_flag(bufferInfo.bufferUsage, rhi::ResourceUsage::UNIFORM_BUFFER))
		LOG_FATAL("VulkanRHI::bind_uniform_buffer(): Can't bind buffer without ResourceUsage::UNIFORM_BUFFER")

	if (!size)
		size = bufferInfo.size;
	
	_descriptorManager->allocate_uniform_buffer(get_vk_obj(buffer), size, offset, slot, _currentImageIndex);
}

void VulkanRHI::begin_command_buffer(rhi::CommandBuffer* cmd, rhi::QueueType queueType)
{
	cmd->handle = _cmdManager->get_command_buffer(queueType);
	cmd->queueType = queueType;
}

void VulkanRHI::wait_command_buffer(rhi::CommandBuffer* cmd, rhi::CommandBuffer* waitForCmd)
{
	VulkanCommandBuffer* cmd1 = static_cast<VulkanCommandBuffer*>(cmd->handle);
	VulkanCommandBuffer* cmd2 = static_cast<VulkanCommandBuffer*>(waitForCmd->handle);
	cmd1->wait_for_cmd(cmd2);
}

void VulkanRHI::submit(rhi::QueueType queueType, bool waitAfterSubmitting)
{
	if (waitAfterSubmitting)
	{
		_cmdManager->submit(queueType, false);
		_cmdManager->wait_fences();
	}
	else
	{
		_cmdManager->submit(queueType, true);
	}
}

void VulkanRHI::present()
{
	_device->get_graphics_queue()->present(_activeSwapChains);
	_activeSwapChains.clear();
}

void VulkanRHI::wait_fences()
{
	_cmdManager->wait_fences();
}

void VulkanRHI::copy_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* srcBuffer, rhi::Buffer* dstBuffer, uint32_t size, uint32_t srcOffset, uint32_t dstOffset)
{
	assert(cmd->handle && srcBuffer->handle && dstBuffer->handle);
	
	if (!has_flag(srcBuffer->bufferInfo.bufferUsage, rhi::ResourceUsage::TRANSFER_SRC))
		LOG_FATAL("VulkanRHI::copy_buffer(): Source buffer doesn't have TRANSFER_SRC usage")
		
	if (!has_flag(dstBuffer->bufferInfo.bufferUsage, rhi::ResourceUsage::TRANSFER_DST))
		LOG_FATAL("VulkanRHI::copy_buffer(): Destination buffer doesn't have TRANSFER_DST usage")
	
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanBuffer* vkSrcBuffer = get_vk_obj(srcBuffer);
	VulkanBuffer* vkDstBuffer = get_vk_obj(dstBuffer);

	VkBufferCopy2 copy{};
	copy.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
	copy.srcOffset = srcOffset;
	copy.dstOffset = dstOffset;
	copy.size = size ? size : srcBuffer->bufferInfo.size;

	VkCopyBufferInfo2 vkBufferInfo{};
	vkBufferInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
	vkBufferInfo.srcBuffer = vkSrcBuffer->get_handle();
	vkBufferInfo.dstBuffer = vkDstBuffer->get_handle();
	vkBufferInfo.regionCount = 1;
	vkBufferInfo.pRegions = &copy;
	vkCmdCopyBuffer2(vkCmd->get_handle(), &vkBufferInfo);
}

void VulkanRHI::copy_texture(rhi::CommandBuffer* cmd, rhi::Texture* srcTexture, rhi::Texture* dstTexture)
{
	assert(cmd->handle && srcTexture->handle && dstTexture->handle);

	if (!has_flag(srcTexture->textureInfo.textureUsage, rhi::ResourceUsage::TRANSFER_SRC))
		LOG_FATAL("VulkanRHI::copy_texture(): Source texture doesn't have TRANSFER_SRC usage")
		
	if (!has_flag(dstTexture->textureInfo.textureUsage, rhi::ResourceUsage::TRANSFER_DST))
		LOG_FATAL("VulkanRHI::copy_texture(): Destination texture doesn't have TRANSFER_DST usage")

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanTexture* vkSrcTexture = get_vk_obj(srcTexture);
	VulkanTexture* vkDstTexture = get_vk_obj(dstTexture);

	VkImageCopy2 copy{};
	copy.sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2;
	copy.extent.width = dstTexture->textureInfo.width;
	copy.extent.height = dstTexture->textureInfo.height;
	copy.extent.depth = std::max(1u, dstTexture->textureInfo.depth);
	copy.srcOffset = { 0, 0, 0 };
	copy.dstOffset = { 0, 0, 0 };

	copy.srcSubresource.aspectMask = get_image_aspect(srcTexture->textureInfo.textureUsage);
	copy.srcSubresource.baseArrayLayer = 0;
	copy.srcSubresource.layerCount = srcTexture->textureInfo.layersCount;
	copy.srcSubresource.mipLevel = 0;

	copy.dstSubresource.aspectMask = get_image_aspect(dstTexture->textureInfo.textureUsage);
	copy.dstSubresource.baseArrayLayer = 0;
	copy.dstSubresource.layerCount = dstTexture->textureInfo.layersCount;
	copy.dstSubresource.mipLevel = 0;

	VkCopyImageInfo2 copyImageInfo{};
	copyImageInfo.sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2;
	copyImageInfo.srcImage = vkSrcTexture->get_handle();
	copyImageInfo.dstImage = vkDstTexture->get_handle();
	copyImageInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	copyImageInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	copyImageInfo.regionCount = 1;
	copyImageInfo.pRegions = &copy;

	vkCmdCopyImage2(vkCmd->get_handle(), &copyImageInfo);
}

void VulkanRHI::blit_texture(rhi::CommandBuffer* cmd, rhi::Texture* srcTexture, rhi::Texture* dstTexture, const std::array<int32_t, 3>& srcOffset, const std::array<int32_t, 3>& dstOffset, uint32_t srcMipLevel, uint32_t dstMipLevel, uint32_t srcBaseLayer, uint32_t dstBaseLayer)
{
	assert(cmd->handle && srcTexture->handle && dstTexture->handle);
	
	if (!has_flag(srcTexture->textureInfo.textureUsage, rhi::ResourceUsage::TRANSFER_SRC))
		LOG_FATAL("VulkanRHI::blit_texture(): Source buffer doesn't have TRANSFER_SRC usage")
	if (!has_flag(dstTexture->textureInfo.textureUsage, rhi::ResourceUsage::TRANSFER_DST))
		LOG_FATAL("VulkanRHI::blit_texture(): Destination buffer doesn't have TRANSFER_DST usage")

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanTexture* vkSrcTexture = get_vk_obj(srcTexture);
	VulkanTexture* vkDstTexture = get_vk_obj(dstTexture);
	rhi::TextureInfo& srcInfo = srcTexture->textureInfo;
	rhi::TextureInfo& dstInfo = dstTexture->textureInfo;

	VkImageBlit2 imageBlit{};
	imageBlit.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
	imageBlit.srcOffsets[1].x = srcOffset[0];
	imageBlit.srcOffsets[1].y = srcOffset[1];
	imageBlit.srcOffsets[1].z = srcOffset[2];
	imageBlit.dstOffsets[1].x = dstOffset[0];
	imageBlit.dstOffsets[1].y = dstOffset[1];
	imageBlit.dstOffsets[1].z = dstOffset[2];
	
	imageBlit.srcSubresource.aspectMask = get_image_aspect(srcInfo.textureUsage);
	imageBlit.srcSubresource.layerCount = srcInfo.layersCount;
	imageBlit.srcSubresource.baseArrayLayer = srcBaseLayer;
	imageBlit.srcSubresource.mipLevel = srcMipLevel;
	imageBlit.dstSubresource.aspectMask = get_image_aspect(dstInfo.textureUsage);
	imageBlit.dstSubresource.layerCount = dstInfo.layersCount;
	imageBlit.dstSubresource.baseArrayLayer = dstBaseLayer;
	imageBlit.dstSubresource.mipLevel = dstMipLevel;

	VkBlitImageInfo2 blitImageInfo{};
	blitImageInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
	blitImageInfo.srcImage = vkSrcTexture->get_handle();
	blitImageInfo.dstImage = vkDstTexture->get_handle();
	blitImageInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blitImageInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blitImageInfo.regionCount = 1;
	blitImageInfo.pRegions = &imageBlit;
	blitImageInfo.filter = VK_FILTER_LINEAR;
	
	vkCmdBlitImage2(vkCmd->get_handle(), &blitImageInfo);
}

void VulkanRHI::copy_buffer_to_texture(rhi::CommandBuffer* cmd, rhi::Buffer* srcBuffer, rhi::Texture* dstTexture)
{
	assert(cmd->handle && srcBuffer->handle && dstTexture->handle);
	
	if (!has_flag(srcBuffer->bufferInfo.bufferUsage, rhi::ResourceUsage::TRANSFER_SRC))
		LOG_FATAL("VulkanRHI::copy_buffer_to_texture(): Source buffer doesn't have TRANSFER_SRC usage")
	if (!has_flag(dstTexture->textureInfo.textureUsage, rhi::ResourceUsage::TRANSFER_DST))
		LOG_FATAL("VulkanRHI::copy_buffer_to_texture(): Destination buffer doesn't have TRANSFER_DST usage")

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanBuffer* vkBuffer = get_vk_obj(srcBuffer);
	VulkanTexture* vkTexture = get_vk_obj(dstTexture);
	rhi::TextureInfo& texInfo = dstTexture->textureInfo;

	VkImageMemoryBarrier2 imageBarrier{};
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
	imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrier.srcAccessMask = 0;
	imageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
	imageBarrier.image = vkTexture->get_handle();
	imageBarrier.subresourceRange = {
		get_image_aspect(texInfo.textureUsage),
		0,
		texInfo.mipLevels,
		0,
		texInfo.layersCount };

	VkDependencyInfo dependencyInfo{};
	dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependencyInfo.imageMemoryBarrierCount = 1;
	dependencyInfo.pImageMemoryBarriers = &imageBarrier;

	vkCmdPipelineBarrier2(vkCmd->get_handle(), &dependencyInfo);
	
	VkBufferImageCopy2 copyRegion{};
	copyRegion.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;
	copyRegion.imageExtent = vkTexture->get_extent();
	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = texInfo.layersCount;

	VkCopyBufferToImageInfo2 copyBufferToImageInfo{};
	copyBufferToImageInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2;
	copyBufferToImageInfo.regionCount = 1;
	copyBufferToImageInfo.pRegions = &copyRegion;
	copyBufferToImageInfo.srcBuffer = vkBuffer->get_handle();
	copyBufferToImageInfo.dstImage = vkTexture->get_handle();
	copyBufferToImageInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	
	vkCmdCopyBufferToImage2(vkCmd->get_handle(), &copyBufferToImageInfo);
	
	imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrier.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
	imageBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
	imageBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

	vkCmdPipelineBarrier2(vkCmd->get_handle(), &dependencyInfo);
}

void VulkanRHI::copy_texture_to_buffer(rhi::CommandBuffer* cmd, rhi::Texture* srcTexture, rhi::Buffer* dstBuffer)
{
	assert(cmd->handle && srcTexture->handle && dstBuffer->handle);
	
	if (!has_flag(srcTexture->textureInfo.textureUsage, rhi::ResourceUsage::TRANSFER_SRC))
		LOG_FATAL("VulkanRHI::copy_texture_to_buffer(): Source texture doesn't have TRANSFER_SRC usage")
	if (!has_flag(dstBuffer->bufferInfo.bufferUsage, rhi::ResourceUsage::TRANSFER_DST))
		LOG_FATAL("VulkanRHI::copy_texture_to_buffer(): Destination buffer doesn't have TRANSFER_DST usage")

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanTexture* vkTexture = get_vk_obj(srcTexture);
	VulkanBuffer* vkBuffer = get_vk_obj(dstBuffer);
	
	VkCopyImageToBufferInfo2 copyImageToBufferInfo{};
	copyImageToBufferInfo.sType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2;
	copyImageToBufferInfo.srcImage = vkTexture->get_handle();
	copyImageToBufferInfo.dstBuffer = vkBuffer->get_handle();
	copyImageToBufferInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	copyImageToBufferInfo.regionCount = 1;

	VkBufferImageCopy2 copy{};
	copy.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
	copy.imageSubresource.aspectMask = get_image_aspect(srcTexture->textureInfo.textureUsage);
	copy.imageSubresource.baseArrayLayer = 0;
	copy.imageSubresource.layerCount = srcTexture->textureInfo.layersCount;
	const uint32_t dataStride = rhi::Utils::get_format_stride(srcTexture->textureInfo.format);
	uint32_t mipWidth = srcTexture->textureInfo.width;
	uint32_t mipHeight = srcTexture->textureInfo.height;
	uint32_t mipDepth = std::max(1u, srcTexture->textureInfo.depth);
	for (uint32_t mipLevel = 0; mipLevel < srcTexture->textureInfo.mipLevels; ++mipLevel)
	{
		copy.imageExtent = { mipWidth, mipHeight, mipDepth };
		copy.imageSubresource.mipLevel = mipLevel;
		
		copyImageToBufferInfo.pRegions = &copy;
		vkCmdCopyImageToBuffer2(vkCmd->get_handle(), &copyImageToBufferInfo);

		copy.bufferOffset += mipWidth * mipHeight * mipDepth * dataStride;
		mipWidth = std::max(1u, mipWidth / 2);
		mipHeight = std::max(1u, mipHeight / 2);
		mipDepth = std::max(1u, mipDepth / 2);
	}
}

void VulkanRHI::set_viewports(rhi::CommandBuffer* cmd, std::vector<rhi::Viewport>& viewports)
{
	assert(cmd && cmd->handle);
	
	VkViewport vulkanViewports[MAX_VIEWPORT_COUNT];
	if (viewports.size() > MAX_VIEWPORT_COUNT)
		LOG_FATAL("VulkanRHI::set_viewports(): You want to set {} viewports if there cannot be more than {} viewports", viewports.size(), MAX_VIEWPORT_COUNT)

	for (uint32_t i = 0; i != viewports.size(); ++i)
	{
		rhi::Viewport& viewport = viewports[i];
		vulkanViewports[i].x = viewport.x;
		vulkanViewports[i].y = viewport.height;
		vulkanViewports[i].width = viewport.width;
		vulkanViewports[i].height = -(int32_t)viewport.height;
		vulkanViewports[i].minDepth = viewport.minDepth;
		vulkanViewports[i].maxDepth = viewport.maxDepth;
	}

	auto vulkanCmd = get_vk_obj(cmd);
	vkCmdSetViewport(vulkanCmd->get_handle(), 0, viewports.size(), vulkanViewports);
}

void VulkanRHI::set_scissors(rhi::CommandBuffer* cmd, std::vector<rhi::Scissor>& scissors)
{
	VkRect2D vulkanScissors[MAX_SCISSOR_COUNT];
	if (scissors.size() > MAX_SCISSOR_COUNT)
		LOG_FATAL("VulkanRHI::set_scissors(): You want to set {} scissors if there cannot be more than {} scissors", scissors.size(), MAX_SCISSOR_COUNT)

	for (uint32_t i = 0; i != scissors.size(); ++i)
	{
		rhi::Scissor& scissor = scissors[i];
		vulkanScissors[i].extent.width = abs(scissor.right - scissor.left);
		vulkanScissors[i].extent.height = abs(scissor.bottom - scissor.top);
		vulkanScissors[i].offset.x = std::max(0, scissor.left);
		vulkanScissors[i].offset.y = std::max(0, scissor.top);
	}

	auto vulkanCmd = get_vk_obj(cmd);
	vkCmdSetScissor(vulkanCmd->get_handle(), 0, scissors.size(), vulkanScissors);
}

void VulkanRHI::push_constants(rhi::CommandBuffer* cmd, rhi::Pipeline* pipeline, void* data)
{
	VulkanPipeline* vkPipeline = get_vk_obj(pipeline);
	vkPipeline->push_constants(get_vk_obj(cmd)->get_handle(), data);
}

void VulkanRHI::bind_vertex_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer)
{
	assert(cmd->handle && buffer->handle);
	
	if (!has_flag(buffer->bufferInfo.bufferUsage, rhi::ResourceUsage::VERTEX_BUFFER))
		LOG_FATAL("VulkanRHI::bind_vertex_buffer(): Buffer wasn't created with VERTEX_BUFFER usage")
	
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanBuffer* vkBuffer = get_vk_obj(buffer);

	VkDeviceSize offset = 0;
	VkBuffer vkBufferHandle = vkBuffer->get_handle();
	vkCmdBindVertexBuffers(vkCmd->get_handle(), 0, 1, &vkBufferHandle, &offset);
}

void VulkanRHI::bind_index_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer)
{
	assert(cmd->handle && buffer->handle);
	
	if (!has_flag(buffer->bufferInfo.bufferUsage, rhi::ResourceUsage::INDEX_BUFFER))
		LOG_FATAL("VulkanRHI::bind_index_buffer(): Buffer wasn't created with INDEX_BUFFER usage")
	
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanBuffer* vkBuffer = get_vk_obj(buffer);

	VkDeviceSize offset = 0;
	vkCmdBindIndexBuffer(vkCmd->get_handle(), vkBuffer->get_handle(), offset, VK_INDEX_TYPE_UINT32);
}

void VulkanRHI::bind_pipeline(rhi::CommandBuffer* cmd, rhi::Pipeline* pipeline)
{
	assert(cmd->handle && pipeline->handle);
	
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanPipeline* vkPipeline = get_vk_obj(pipeline);
	vkPipeline->bind(vkCmd->get_handle(), _currentImageIndex);
}

void VulkanRHI::begin_render_pass(rhi::CommandBuffer* cmd, rhi::RenderPass* renderPass, rhi::ClearValues& clearValues)
{
	assert(cmd->handle && renderPass->handle);

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanRenderPass* pass = get_vk_obj(renderPass);

	VkRenderPassBeginInfo beginInfo = pass->get_begin_info(clearValues, _currentImageIndex);

	vkCmdBeginRenderPass(vkCmd->get_handle(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRHI::end_render_pass(rhi::CommandBuffer* cmd)
{
	assert(cmd->handle);
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	vkCmdEndRenderPass(vkCmd->get_handle());
}

void VulkanRHI::begin_rendering(rhi::CommandBuffer* cmd, rhi::RenderingBeginInfo* beginInfo)
{
	assert(cmd->handle && beginInfo);

	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;

	if (has_flag(beginInfo->flags, rhi::RenderingBeginInfoFlags::RESUMING))
	{
		renderingInfo.flags |= VK_RENDERING_RESUMING_BIT;
	}
	if (has_flag(beginInfo->flags, rhi::RenderingBeginInfoFlags::SUSPENDING))
	{
		renderingInfo.flags |= VK_RENDERING_SUSPENDING_BIT;
	}
	
	renderingInfo.layerCount = 1;
	if (beginInfo->multiviewInfo.isEnabled)
	{
		renderingInfo.viewMask = (1 << beginInfo->multiviewInfo.viewCount) - 1;
	}
	
	renderingInfo.renderArea.offset.x = 0;
	renderingInfo.renderArea.offset.y = 0;
	if (!beginInfo->renderTargets.empty())
	{
		VulkanTexture* vulkanTexture = get_vk_obj(beginInfo->renderTargets[0].target->texture);
		VkExtent3D extent3D = vulkanTexture->get_extent();
		renderingInfo.renderArea.extent = { extent3D.width, extent3D.height };
	}
	else
	{
		renderingInfo.renderArea.extent = { _mainSwapChain->get_width(), _mainSwapChain->get_height() };
	}
	renderingInfo.colorAttachmentCount = 0;
	renderingInfo.pColorAttachments = nullptr;
	renderingInfo.pDepthAttachment = nullptr;
	renderingInfo.pStencilAttachment = nullptr;

	std::vector<VkRenderingAttachmentInfo> colorAttachments;
	VkRenderingAttachmentInfo depthAttachment{};
	VkRenderingAttachmentInfo stencilAttachment{};

	for (auto& renderTarget : beginInfo->renderTargets)
	{
		rhi::TextureView* textureView = renderTarget.target;
		rhi::TextureInfo& textureInfo = textureView->texture->textureInfo;

		if (has_flag(textureInfo.textureUsage, rhi::ResourceUsage::COLOR_ATTACHMENT))
		{
			VkRenderingAttachmentInfo& attachmentInfo = colorAttachments.emplace_back();
			attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			attachmentInfo.pNext = nullptr;
			attachmentInfo.storeOp = get_attach_store_op(renderTarget.storeOp);
			attachmentInfo.loadOp = get_attach_load_op(renderTarget.loadOp);
			attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentInfo.imageView = get_vk_obj(textureView)->get_handle();
			std::array<float, 4>& clearVal = renderTarget.clearValue.color;
			attachmentInfo.clearValue.color = { { clearVal[0], clearVal[1], clearVal[2], clearVal[3] } };
		}
		if (has_flag(textureInfo.textureUsage, rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT))
		{
			if (textureInfo.format != rhi::Format::S8_UINT)
			{
				depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				depthAttachment.imageView = get_vk_obj(textureView)->get_handle();
				depthAttachment.loadOp = get_attach_load_op(renderTarget.loadOp);
				depthAttachment.storeOp = get_attach_store_op(renderTarget.storeOp);
				depthAttachment.clearValue.depthStencil.depth = renderTarget.clearValue.depthStencil.depth;

				if (textureInfo.format != rhi::Format::D16_UNORM || textureInfo.format != rhi::Format::D32_SFLOAT)
				{
					depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					depthAttachment.clearValue.depthStencil.stencil = renderTarget.clearValue.depthStencil.stencil;
				}
				else
				{
					depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				}

				renderingInfo.pDepthAttachment = &depthAttachment;
			}
			else
			{
				stencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				stencilAttachment.imageView = get_vk_obj(textureView)->get_handle();
				stencilAttachment.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
				stencilAttachment.loadOp = get_attach_load_op(renderTarget.loadOp);
				stencilAttachment.storeOp = get_attach_store_op(renderTarget.storeOp);
				stencilAttachment.clearValue.depthStencil.stencil = renderTarget.clearValue.depthStencil.stencil;
				renderingInfo.pStencilAttachment = &stencilAttachment;
			}
		}
	}
	
	if (!colorAttachments.empty())
	{
		renderingInfo.colorAttachmentCount = colorAttachments.size();
		renderingInfo.pColorAttachments = colorAttachments.data();
	}
	vkCmdBeginRendering(get_vk_obj(cmd)->get_handle(), &renderingInfo);
}

void VulkanRHI::begin_rendering(rhi::CommandBuffer* cmd, rhi::SwapChain* swapChain, rhi::ClearValues* clearValues)
{
	assert(cmd->handle && clearValues);
	
	VulkanSwapChain* vkSwapChain = swapChain ? get_vk_obj(swapChain) : _mainSwapChain.get();
	_activeSwapChains.push_back(vkSwapChain);
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	vkCmd->add_wait_semaphore(vkSwapChain->get_acquire_semaphore());
	vkSwapChain->prepare_for_drawing(vkCmd);
		
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.pDepthAttachment = nullptr;
	renderingInfo.pStencilAttachment = nullptr;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.layerCount = 1;
	renderingInfo.renderArea.offset.x = 0;
	renderingInfo.renderArea.offset.y = 0;
	renderingInfo.renderArea.extent.width = vkSwapChain->get_width();
	renderingInfo.renderArea.extent.height = vkSwapChain->get_height();
	
	VkRenderingAttachmentInfo attachmentInfo{};
	attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentInfo.imageView = vkSwapChain->get_image_view_handle();
	attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentInfo.clearValue.color = { { clearValues->color[0], clearValues->color[1], clearValues->color[2], clearValues->color[3] } };
	
	renderingInfo.pColorAttachments = &attachmentInfo;
	vkCmdBeginRendering(vkCmd->get_handle(), &renderingInfo);
}

void VulkanRHI::end_rendering(rhi::CommandBuffer* cmd)
{
	assert(cmd->handle);
	vkCmdEndRendering(get_vk_obj(cmd)->get_handle());
}

void VulkanRHI::end_rendering(rhi::CommandBuffer* cmd, rhi::SwapChain* swapChain)
{
	assert(cmd->handle);
	vkCmdEndRendering(get_vk_obj(cmd)->get_handle());
	VulkanSwapChain* vkSwapChain = swapChain ? get_vk_obj(swapChain) : _mainSwapChain.get();
	vkSwapChain->prepare_for_presenting(get_vk_obj(cmd));
}

void VulkanRHI::draw(rhi::CommandBuffer* cmd, uint64_t vertexCount)
{
	assert(cmd->handle);
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	vkCmdDraw(vkCmd->get_handle(), vertexCount, 1, 0, 0);
}

void VulkanRHI::draw_indexed(
	rhi::CommandBuffer* cmd,
	uint32_t indexCount,
	uint32_t instanceCount,
	uint32_t firstIndex,
	int32_t vertexOffset,
	uint32_t firstInstance)
{
	assert(cmd->handle);
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	vkCmdDrawIndexed(vkCmd->get_handle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanRHI::draw_indirect(rhi::CommandBuffer* cmd, rhi::Buffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride)
{
	assert(cmd->handle && buffer->handle);
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanBuffer* vkBuffer = get_vk_obj(buffer);
	vkCmdDrawIndirect(vkCmd->get_handle(), vkBuffer->get_handle(), offset, drawCount, stride);
}

void VulkanRHI::draw_indexed_indirect(rhi::CommandBuffer* cmd, rhi::Buffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride)
{
	assert(cmd->handle && buffer->handle);
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanBuffer* vkBuffer = get_vk_obj(buffer);
	vkCmdDrawIndexedIndirect(vkCmd->get_handle(), vkBuffer->get_handle(), offset, drawCount, stride);
}

void VulkanRHI::dispatch(rhi::CommandBuffer* cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	assert(cmd->handle);
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	vkCmdDispatch(vkCmd->get_handle(), groupCountX, groupCountY, groupCountZ);
}

void VulkanRHI::fill_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer, uint32_t dstOffset, uint32_t size, uint32_t data)
{
	assert(cmd->handle && buffer->handle);
	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanBuffer* vkBuffer = get_vk_obj(buffer);
	vkCmdFillBuffer(vkCmd->get_handle(), vkBuffer->get_handle(), dstOffset, size, data);
}

void VulkanRHI::add_pipeline_barriers(rhi::CommandBuffer* cmd, const std::vector<rhi::PipelineBarrier>& barriers)
{
	assert(cmd->handle && !barriers.empty());
	
	std::vector<VkMemoryBarrier2> memoryBarriers;
	std::vector<VkBufferMemoryBarrier2> bufferBarriers;
	std::vector<VkImageMemoryBarrier2> imageBarriers;
	for (auto& pipelineBarrier : barriers)
	{
		switch (pipelineBarrier.get_barrier_type())
		{
			case rhi::PipelineBarrier::BarrierType::MEMORY:
			{
				const rhi::PipelineBarrier::MemoryBarrier& rhiMemoryBarrier = pipelineBarrier.get_memory_barrier();
				VkMemoryBarrier2 memoryBarrier{};
				memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
				memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				memoryBarrier.srcAccessMask = get_access(rhiMemoryBarrier.srcLayout);
				memoryBarrier.dstAccessMask = get_access(rhiMemoryBarrier.dstLayout);
				memoryBarriers.push_back(memoryBarrier);
				break;
			}
			case rhi::PipelineBarrier::BarrierType::BUFFER:
			{
				const rhi::PipelineBarrier::BufferBarrier& rhiBufferBarrier = pipelineBarrier.get_buffer_barrier();
				VkBufferMemoryBarrier2 bufferBarrier{};
				bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
				const rhi::Buffer* buffer = rhiBufferBarrier.buffer;
				bufferBarrier.buffer = get_vk_obj(buffer)->get_handle();
				bufferBarrier.offset = 0;
				if (has_flag(buffer->bufferInfo.bufferUsage, rhi::ResourceUsage::STORAGE_BUFFER))
				{
					bufferBarrier.size = VK_WHOLE_SIZE;
				}
				else
				{
					bufferBarrier.size = buffer->bufferInfo.size;
				}
				bufferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				bufferBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				bufferBarrier.srcAccessMask = get_access(rhiBufferBarrier.srcLayout);
				bufferBarrier.dstAccessMask = get_access(rhiBufferBarrier.dstLayout);
				bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferBarriers.push_back(bufferBarrier);
				break;
			}
			case rhi::PipelineBarrier::BarrierType::TEXTURE:
			{
				VkImageMemoryBarrier2 imageBarrier{};
				const rhi::PipelineBarrier::TextureBarrier& rhiTextureBarrier = pipelineBarrier.get_texture_barrier();
				imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
				imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imageBarrier.srcAccessMask = get_access(rhiTextureBarrier.srcLayout);
				imageBarrier.dstAccessMask = get_access(rhiTextureBarrier.dstLayout);
				imageBarrier.oldLayout = get_image_layout(rhiTextureBarrier.srcLayout);
				imageBarrier.newLayout = get_image_layout(rhiTextureBarrier.dstLayout);
				VulkanTexture* vkTexture = get_vk_obj(rhiTextureBarrier.texture);
				imageBarrier.image = vkTexture->get_handle();
				VkImageSubresourceRange range;
				if (has_flag(rhiTextureBarrier.texture->textureInfo.textureUsage, rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT))
				{
					range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				}
				else
				{
					range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				}
				range.layerCount = !rhiTextureBarrier.layerCount ? VK_REMAINING_ARRAY_LAYERS : rhiTextureBarrier.layerCount;
				range.baseArrayLayer = rhiTextureBarrier.baseLayer;
				range.levelCount = !rhiTextureBarrier.levelCount ? VK_REMAINING_MIP_LEVELS : rhiTextureBarrier.levelCount;
				range.baseMipLevel = rhiTextureBarrier.baseMipLevel;
				imageBarrier.subresourceRange = range;
				imageBarriers.push_back(imageBarrier);
				break;
			}
		}
	}

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VkDependencyInfo dependencyInfo{};
	dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependencyInfo.memoryBarrierCount = memoryBarriers.size();
	dependencyInfo.pMemoryBarriers = memoryBarriers.data();
	dependencyInfo.bufferMemoryBarrierCount = bufferBarriers.size();
	dependencyInfo.pBufferMemoryBarriers = bufferBarriers.data();
	dependencyInfo.imageMemoryBarrierCount = imageBarriers.size();
	dependencyInfo.pImageMemoryBarriers = imageBarriers.data();
	vkCmdPipelineBarrier2(vkCmd->get_handle(), &dependencyInfo);
}

void VulkanRHI::wait_for_gpu()
{
	VK_CHECK(vkDeviceWaitIdle(_device->get_device()));
}

void VulkanRHI::create_query_pool(rhi::QueryPool* queryPool, rhi::QueryPoolInfo* queryPoolInfo)
{
	assert(queryPool && queryPoolInfo);

	queryPool->info = *queryPoolInfo;
	create_query_pool(queryPool);
}

void VulkanRHI::create_query_pool(rhi::QueryPool* queryPool)
{
	assert(queryPool);
	
	if (queryPool->info.type == rhi::QueryType::UNDEFINED)
		LOG_FATAL("VulkanRHI::create_query_pool(): Can't create query pool with rhi::QueryType == UNDEFINED")
	if (!queryPool->info.queryCount)
		LOG_FATAL("VulkanRHI::create_query_pool(): Can't create query pool with query count = 0")

	queryPool->handle = _vkObjectPool.allocate<VulkanQueryPool>(_device.get(), &queryPool->info);
}

void VulkanRHI::begin_query(const rhi::CommandBuffer* cmd, const rhi::QueryPool* queryPool, uint32_t queryIndex)
{
	assert(cmd->handle && queryPool->handle);

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanQueryPool* vkQueryPool = get_vk_obj(queryPool);
	
	switch (queryPool->info.type)
	{
		case rhi::QueryType::OCCLUSION:
		case rhi::QueryType::PIPELINE_STATISTICS:
			vkCmdBeginQuery(vkCmd->get_handle(), vkQueryPool->get_handle(), queryIndex, 0);
			break;
		case rhi::QueryType::BINARY_OCCLUSION:
			vkCmdBeginQuery(vkCmd->get_handle(), vkQueryPool->get_handle(), queryIndex, VK_QUERY_CONTROL_PRECISE_BIT);
			break;
		case rhi::QueryType::TIMESTAMP:
			break;
	}
}

void VulkanRHI::end_query(const rhi::CommandBuffer* cmd, const rhi::QueryPool* queryPool, uint32_t queryIndex)
{
	assert(cmd->handle && queryPool->handle);

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanQueryPool* vkQueryPool = get_vk_obj(queryPool);

	switch (queryPool->info.type)
	{
		case rhi::QueryType::OCCLUSION:
		case rhi::QueryType::BINARY_OCCLUSION:
		case rhi::QueryType::PIPELINE_STATISTICS:
			vkCmdEndQuery(vkCmd->get_handle(), vkQueryPool->get_handle(), queryIndex);
			break;
		case rhi::QueryType::TIMESTAMP:
			vkCmdWriteTimestamp2(vkCmd->get_handle(), VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, vkQueryPool->get_handle(), queryIndex);
			break;
	}
}

void VulkanRHI::get_query_pool_result(
	const rhi::QueryPool* queryPool,
	std::vector<uint64_t>& outputData,
	uint32_t queryIndex,
	uint32_t queryCount,
	uint32_t stride)
{
	assert(queryPool->handle);

	VulkanQueryPool* vkQueryPool = get_vk_obj(queryPool);
	outputData.resize(queryCount);
	VK_CHECK(vkGetQueryPoolResults(
		_device->get_device(),
		vkQueryPool->get_handle(),
		queryIndex,
		queryCount,
		outputData.size() * sizeof(uint64_t),
		outputData.data(),
		stride,
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));
}

void VulkanRHI::reset_query(const rhi::CommandBuffer* cmd, const rhi::QueryPool* queryPool, uint32_t queryIndex, uint32_t queryCount)
{
	assert(cmd->handle && queryPool->handle);

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanQueryPool* vkQueryPool = get_vk_obj(queryPool);

	vkCmdResetQueryPool(vkCmd->get_handle(), vkQueryPool->get_handle(), queryIndex, queryCount);
}

void VulkanRHI::copy_query_pool_results(
	const rhi::CommandBuffer* cmd,
	const rhi::QueryPool* queryPool,
	uint32_t firstQuery,
	uint32_t queryCount,
	uint32_t stride,
	const rhi::Buffer* dstBuffer,
	uint32_t dstOffset)
{
	assert(cmd->handle && queryPool->handle && dstBuffer->handle);

	if (cmd->queueType != rhi::QueueType::GRAPHICS && cmd->queueType != rhi::QueueType::COMPUTE)
		LOG_FATAL("VulkanRHI::copy_query_pool_results(): Can't copy query results using not graphics or compute command buffer")

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanQueryPool* vkQueryPool = get_vk_obj(queryPool);
	VulkanBuffer* vkBuffer = get_vk_obj(dstBuffer);

	vkCmdCopyQueryPoolResults(
		vkCmd->get_handle(),
		vkQueryPool->get_handle(),
		firstQuery,
		queryCount,
		vkBuffer->get_handle(),
		dstOffset,
		sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
}

rhi::GPUMemoryUsage VulkanRHI::get_memory_usage()
{
	rhi::GPUMemoryUsage memoryUsage;
	VmaBudget budgets[VK_MAX_MEMORY_HEAPS];
	vmaGetHeapBudgets(_device->get_allocator(), budgets);
	const VkPhysicalDeviceMemoryProperties& memoryProperties = _device->get_memory_properties().memoryProperties;
	for (auto i = 0; i != memoryProperties.memoryHeapCount; ++i)
	{
		if (memoryProperties.memoryHeaps[i].flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			memoryUsage.total += budgets[i].budget;
			memoryUsage.usage += budgets[i].usage;
		}
	}
	return memoryUsage;
}

void VulkanRHI::print_gpu_info()
{
	const VkPhysicalDeviceMemoryProperties& memoryProperties = _device->get_memory_properties().memoryProperties;
	VmaBudget budgets[VK_MAX_MEMORY_HEAPS];
	vmaGetHeapBudgets(_device->get_allocator(), budgets);
	for (auto i = 0; i != memoryProperties.memoryHeapCount; ++i)
	{
		if (memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
		{
			LOG_INFO("VulkanRHI::init(): GPU heap budget: {}", budgets[i].budget)
		}
	}

	const auto& properties12 = _device->get_physical_device_vulkan_1_2_properties();
	const auto& properties11 = _device->get_physical_device_vulkan_1_1_properties();
	LOG_INFO("VulkanRHI::init(): Max samplers: {}", properties12.maxDescriptorSetUpdateAfterBindSamplers)
	LOG_INFO("VulkanRHI::init(): Max sampled images: {}", properties12.maxDescriptorSetUpdateAfterBindSampledImages);
	LOG_INFO("VulkanRHI::init(): Max storage images: {}", properties12.maxDescriptorSetUpdateAfterBindStorageImages)
	LOG_INFO("VulkanRHI::init(): Max storage buffers: {}", properties12.maxDescriptorSetUpdateAfterBindStorageBuffers)
	LOG_INFO("VulkanRHI::init(): Max multiview count: {}", properties11.maxMultiviewViewCount)
}

void VulkanRHI::recreate_swap_chain()
{
	vkDeviceWaitIdle(_device->get_device());
	_mainSwapChain->recreate();
	//_attachmentManager.recreate_attachments(_device.get(), _descriptorManager.get(), _mainSwapChain->get_width(), _mainSwapChain->get_height());
}
