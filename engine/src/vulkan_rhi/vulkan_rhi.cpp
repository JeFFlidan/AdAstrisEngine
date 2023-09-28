#define NOMINMAX
#include "vulkan_rhi.h"
#include "vulkan_queue.h"
#include "vulkan_common.h"

#include "profiler/logger.h"
#include "file_system/utils.h"
#include <vkbootstrap/VkBootstrap.h>

#ifndef VMA_IMPLEMENTATION
	#define VMA_IMPLEMENTATION
#endif
#include <vma/vk_mem_alloc.h>

#include <algorithm>

using namespace ad_astris;

constexpr uint32_t MAX_VIEWPORT_COUNT = 16;
constexpr uint32_t MAX_SCISSOR_COUNT = 16;

void vulkan::VulkanRHI::init(rhi::RHIInitContext& initContext)
{
	assert(initContext.window != nullptr);
	assert(initContext.fileSystem != nullptr);
	assert(initContext.swapChainInfo != nullptr);
	
	_fileSystem = initContext.fileSystem;
	_mainWindow = initContext.window;
	vkb::Instance vkbInstance = create_instance();
	_instance = vkbInstance.instance;
	_debugMessenger = vkbInstance.debug_messenger;
	_device = std::make_unique<VulkanDevice>(vkbInstance, initContext.window);
	
	_swapChain = std::make_unique<VulkanSwapChain>(initContext.swapChainInfo, _device.get());
	_cmdManager = std::make_unique<VulkanCommandManager>(_device.get(), _swapChain.get());
	
	_descriptorManager = std::make_unique<VulkanDescriptorManager>(_device.get(), _swapChain->get_buffers_count());
	_pipelineLayoutCache = std::make_unique<VulkanPipelineLayoutCache>(_device.get(), _descriptorManager.get());
	create_allocator();
	create_pipeline_cache();

	auto& properties = _device->get_physical_device_vulkan_1_2_properties();
	auto& properties11 = _device->get_physical_device_vulkan_1_1_properties();
	LOG_INFO("MAX SAMPLERS: {}", properties.maxDescriptorSetUpdateAfterBindSamplers)
	LOG_INFO("MAX IMAGES: {}", properties.maxDescriptorSetUpdateAfterBindSampledImages);
	LOG_INFO("MAX STORAGE IMAGES: {}", properties.maxDescriptorSetUpdateAfterBindStorageImages)
	LOG_INFO("MAX STORAGE BUFFERS: {}", properties.maxDescriptorSetUpdateAfterBindStorageBuffers)

	_attachmentDescPool.allocate_new_pool(64);
	//LOG_INFO("TEST: {}", properties11.subgroupSize);
}

// TODO Must test it
void vulkan::VulkanRHI::cleanup()
{
	_cmdManager->wait_all_fences();
	save_pipeline_cache();
	
	for (auto& pipeline : _vulkanPipelines)
		pipeline->cleanup();
	
	for (auto& shader : _vulkanShaders)
		shader->cleanup();
	
	for (auto& renderPass : _vulkanRenderPasses)
		renderPass->cleanup();
	
	for (auto& sampler : _vulkanSamplers)
		sampler->destroy(_device.get());
	
	for (auto& textureView : _vulkanTextureViews)
		textureView->destroy(_device.get());
	
	for (auto& texture : _vulkanTextures)
		texture->destroy_texture(_allocator);
	
	for (auto& buffer : _vulkanBuffers)
		buffer->destroy_buffer(&_allocator);
	
	VkDevice device = _device.get()->get_device();
	vkDestroyPipelineCache(device, _pipelineCache, nullptr);
	vmaDestroyAllocator(_allocator);
	_pipelineLayoutCache->cleanup();
	_descriptorManager->cleanup();
	_cmdManager->cleanup();
	_attachmentDescPool.cleanup();
	_device->cleanup();
	vkb::destroy_debug_utils_messenger(_instance, _debugMessenger, nullptr);
	vkDestroyInstance(_instance, nullptr);
}

void vulkan::VulkanRHI::create_swap_chain(rhi::SwapChain* swapChain, rhi::SwapChainInfo* info)
{
	if (!swapChain || !info)
	{
		LOG_ERROR("VulkanRHI::create_swap_chain(): Invalid pointers")
		return;
	}
	_swapChain = std::make_unique<VulkanSwapChain>(info, _device.get());
	_cmdManager = std::make_unique<VulkanCommandManager>(_device.get(), _swapChain.get());
	swapChain->handle = _swapChain.get();
	swapChain->info = *info;
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

void vulkan::VulkanRHI::get_swap_chain_texture_views(std::vector<rhi::TextureView>& textureViews)
{
	// for (auto& textureView : _swapChain->get_texture_views())
	// 	textureViews.push_back(textureView);
}

bool vulkan::VulkanRHI::acquire_next_image(uint32_t& nextImageIndex, uint32_t currentFrameIndex)
{
	if (!_cmdManager->acquire_next_image(_swapChain.get(), nextImageIndex, currentFrameIndex))
	{
		recreate_swap_chain();
		return false;
	}
	_currentImageIndex = nextImageIndex;
	return true;
}

void vulkan::VulkanRHI::create_buffer(rhi::Buffer* buffer, rhi::BufferInfo* bufInfo, void* data)
{
	if (!buffer)
	{
		LOG_ERROR("Can't create buffer if buffer parameter is invalid")
		return;
	}
	
	buffer->bufferInfo = *bufInfo;
	create_buffer(buffer, data);
}

void vulkan::VulkanRHI::create_buffer(rhi::Buffer* buffer, void* data)
{
	if (!buffer)
	{
		LOG_ERROR("Can't create buffer if buffer parameter is invalid")
		return;
	}
	
	VkBufferUsageFlags bufferUsage = get_buffer_usage(buffer->bufferInfo.bufferUsage);
	if (!bufferUsage)
	{
		LOG_ERROR("Invalid buffer usage")
		return;
	}
	
	VmaMemoryUsage memoryUsage = get_memory_usage(buffer->bufferInfo.memoryUsage);
	if (memoryUsage == VMA_MEMORY_USAGE_UNKNOWN)
	{
		LOG_ERROR("Invalid memory usage (buffer)")
		return;
	}

	auto vulkanBufferTemp = std::make_unique<VulkanBuffer>(&_allocator, buffer->bufferInfo.size, bufferUsage, memoryUsage);
	VulkanBuffer* vulkanBuffer = nullptr;
	{
		std::scoped_lock<std::mutex> locker(_buffersMutex);
		_vulkanBuffers.push_back(std::move(vulkanBufferTemp));
		vulkanBuffer = _vulkanBuffers.back().get();
	}
	
	buffer->data = vulkanBuffer;
	buffer->size = buffer->bufferInfo.size;
	buffer->type = rhi::Resource::ResourceType::BUFFER;

	if (has_flag(buffer->bufferInfo.bufferUsage, rhi::ResourceUsage::STORAGE_BUFFER))
		_descriptorManager->allocate_bindless_descriptor(vulkanBuffer, buffer->size, 0);
	
	if (data == nullptr)
		return;
	if (data != nullptr && memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY)
	{
		LOG_ERROR("Can't copy data from CPU to buffer if memory usage is VMA_MEMORY_USAGE_GPU_ONLY")
		return;
	}
	vulkanBuffer->copy_from(&_allocator, data, buffer->size);
}

void vulkan::VulkanRHI::destroy_buffer(rhi::Buffer* buffer)
{
	VulkanBuffer* vulkanBuffer = get_vk_obj(buffer);
	vulkanBuffer->destroy_buffer(&_allocator);

	std::scoped_lock<std::mutex> locker(_buffersMutex);
	_vulkanBuffers.erase(std::remove_if(_vulkanBuffers.begin(), _vulkanBuffers.end(), [&](auto& object)
	{
		return vulkanBuffer == object.get();
	}), _vulkanBuffers.end());
	buffer->data = nullptr;
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

	texture->textureInfo = *texInfo;
	create_texture(texture);
}

void vulkan::VulkanRHI::create_texture(rhi::Texture* texture)
{
	if (!texture)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Invalid rhi::Texture pointer")
		return;
	}

	rhi::TextureInfo* texInfo = &texture->textureInfo;
	
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
	{
		createInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	VkImageUsageFlags imgUsage = get_image_usage(texInfo->textureUsage);
	createInfo.usage = imgUsage;
	createInfo.imageType = get_image_type(texInfo->textureDimension);

	auto vkTextureTemp = std::make_unique<VulkanTexture>(createInfo, &_allocator, memoryUsage);
	VulkanTexture* vkTexture = nullptr;
	{
		std::scoped_lock<std::mutex> locker(_texturesMutex);
		_vulkanTextures.push_back(std::move(vkTextureTemp));
		vkTexture = _vulkanTextures.back().get();
	}
	if (vkTexture->get_handle() == VK_NULL_HANDLE)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Failed to allocate VkImage")
		return;
	}
	texture->data = vkTexture;
	texture->type = rhi::Resource::ResourceType::TEXTURE;

	if (has_flag(texInfo->textureUsage, rhi::ResourceUsage::COLOR_ATTACHMENT)
		|| has_flag(texInfo->textureUsage, rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT))
	{
		_viewIndicesByTexturePtr[vkTexture] = _attachmentDescPool.allocate();
		_viewIndicesByTexturePtr[vkTexture]->imageCreateInfo = createInfo;
	}
}

void vulkan::VulkanRHI::create_texture_view(rhi::TextureView* textureView, rhi::TextureViewInfo* viewInfo, rhi::Texture* texture)
{
	if (!textureView || !texture)
	{
		LOG_ERROR("Can't create texture view if one of the parameters is invalid")
	}

	textureView->viewInfo = *viewInfo;
	create_texture_view(textureView, texture);
}

void vulkan::VulkanRHI::create_texture_view(rhi::TextureView* textureView, rhi::Texture* texture)
{
	if (!textureView || !texture)
	{
		LOG_ERROR("Can't create texture view if one of the parameters is invalid")
	}

	rhi::TextureViewInfo* viewInfo = &textureView->viewInfo;
	
	rhi::TextureInfo texInfo = texture->textureInfo;
	VulkanTexture* vkTexture = static_cast<VulkanTexture*>(texture->data);
	
	VkFormat format = get_format(texInfo.format);
	VkImageUsageFlags imgUsage = get_image_usage(texInfo.textureUsage);

	VkImageAspectFlags aspectFlags;
	if (viewInfo->textureAspect == rhi::TextureAspect::UNDEFINED)
	{
		if ((imgUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		else
			aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	else
	{
		aspectFlags = get_image_aspect(viewInfo->textureAspect);
	}
	
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
	createInfo.subresourceRange.levelCount = texInfo.mipLevels;
	createInfo.subresourceRange.baseArrayLayer = viewInfo->baseLayer;
	createInfo.subresourceRange.layerCount = texInfo.layersCount;
	createInfo.subresourceRange.aspectMask = aspectFlags;

	auto vkTextureViewTemp = std::make_unique<VulkanTextureView>(_device.get(), createInfo);

	uint32_t vkTextureViewIndex;
	{
		std::scoped_lock<std::mutex> locker(_texturesViewMutex);
		vkTextureViewIndex = _vulkanTextureViews.size();
		_vulkanTextureViews.push_back(std::move(vkTextureViewTemp));
		textureView->handle = _vulkanTextureViews.back().get();
	}
	
	textureView->texture = texture;

	if ((imgUsage & VK_IMAGE_USAGE_SAMPLED_BIT) == VK_IMAGE_USAGE_SAMPLED_BIT)
	{
		_descriptorManager->allocate_bindless_descriptor(get_vk_obj(textureView), TextureDescriptorHeapType::TEXTURES);
	}
	else if ((imgUsage & VK_IMAGE_USAGE_STORAGE_BIT) == VK_IMAGE_USAGE_STORAGE_BIT)
	{
		_descriptorManager->allocate_bindless_descriptor(get_vk_obj(textureView), TextureDescriptorHeapType::STORAGE_TEXTURES);
	}

	if (has_flag(texInfo.textureUsage, rhi::ResourceUsage::COLOR_ATTACHMENT)
		|| has_flag(texInfo.textureUsage, rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT))
	{
		AttachmentDesc::ViewDesc& viewDesc = _viewIndicesByTexturePtr[vkTexture]->viewDescriptions.emplace_back();
		viewDesc.viewArrayIndex = vkTextureViewIndex;
		viewDesc.imageViewCreateInfo = createInfo;
	}
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

	_vulkanSamplers.emplace_back(new VulkanSampler(_device.get(), createInfo));
	sampler->handle = _vulkanSamplers.back().get();
	sampler->sampInfo = *sampInfo;

	_descriptorManager->allocate_bindless_descriptor(_vulkanSamplers.back().get());
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

	auto vulkanShader = std::make_unique<VulkanShader>(_device->get_device());
	vulkanShader->create_shader_module(shaderInfo);
	shader->type = shaderInfo->shaderType;

	{
		std::scoped_lock<std::mutex> locker(_shadersMutex);
		_vulkanShaders.push_back(std::move(vulkanShader));
		VulkanShader* vulkanShader = _vulkanShaders.back().get();
		shader->handle = vulkanShader;
	}
}

void vulkan::VulkanRHI::create_graphics_pipeline(rhi::Pipeline* pipeline, rhi::GraphicsPipelineInfo* info)
{
	if (!pipeline || !info)
	{
		LOG_ERROR("VulkanRHI::create_graphics_pipeline(): Invalid pointers")
		return;
	}
	
	auto vulkanPipeline = std::make_unique<VulkanPipeline>(_device.get(), info, _pipelineCache, _pipelineLayoutCache.get());

	{
		std::scoped_lock<std::mutex> locker(_pipelinesMutex);
		_vulkanPipelines.push_back(std::move(vulkanPipeline));
		pipeline->handle = _vulkanPipelines.back().get();
	}
	
	pipeline->type = rhi::PipelineType::GRAPHICS;
}

void vulkan::VulkanRHI::create_compute_pipeline(rhi::Pipeline* pipeline, rhi::ComputePipelineInfo* info)
{
	if (!pipeline || !info)
	{
		LOG_ERROR("VulkanRHI::create_compute_pipeline(): Invalid pointers")
		return;
	}
	
	auto vulkanPipeline = std::make_unique<VulkanPipeline>(_device.get(), info, _pipelineCache, _pipelineLayoutCache.get());

	{
		std::scoped_lock<std::mutex> locker(_pipelinesMutex);
		_vulkanPipelines.push_back(std::move(vulkanPipeline));
		pipeline->handle = _vulkanPipelines.back().get();
	}
	
	pipeline->type = rhi::PipelineType::COMPUTE;
}

void vulkan::VulkanRHI::create_render_pass(rhi::RenderPass* renderPass, rhi::RenderPassInfo* passInfo)
{
	if (passInfo->renderBuffers.empty())
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

	_vulkanRenderPasses.emplace_back(new VulkanRenderPass(_device.get(), passInfo));
	renderPass->handle = _vulkanRenderPasses.back().get();
}

uint32_t vulkan::VulkanRHI::get_descriptor_index(rhi::Buffer* buffer)
{
	return get_vk_obj(buffer)->get_descriptor_index();
}

uint32_t vulkan::VulkanRHI::get_descriptor_index(rhi::TextureView* textureView)
{
	return get_vk_obj(textureView)->get_descriptor_index();
}

uint32_t vulkan::VulkanRHI::get_descriptor_index(rhi::Sampler* sampler)
{
	return get_vk_obj(sampler)->get_descriptor_index();
}

void vulkan::VulkanRHI::bind_uniform_buffer(rhi::Buffer* buffer, uint32_t slot, uint32_t size, uint32_t offset)
{
	rhi::BufferInfo& bufferInfo = buffer->bufferInfo;
	if (!has_flag(bufferInfo.bufferUsage, rhi::ResourceUsage::UNIFORM_BUFFER))
		LOG_FATAL("VulkanRHI::bind_uniform_buffer(): Can't bind buffer without ResourceUsage::UNIFORM_BUFFER")

	if (!size)
		size = bufferInfo.size;
	
	_descriptorManager->allocate_uniform_buffer(get_vk_obj(buffer), size, offset, slot, _currentImageIndex);
}

// TODO Queue type for command buffer
void vulkan::VulkanRHI::begin_command_buffer(rhi::CommandBuffer* cmd, rhi::QueueType queueType)
{
	cmd->handle = _cmdManager->get_command_buffer(queueType);
}

void vulkan::VulkanRHI::wait_command_buffer(rhi::CommandBuffer* cmd, rhi::CommandBuffer* waitForCmd)
{
	VulkanCommandBuffer* cmd1 = static_cast<VulkanCommandBuffer*>(cmd->handle);
	VulkanCommandBuffer* cmd2 = static_cast<VulkanCommandBuffer*>(waitForCmd->handle);
	_cmdManager->wait_for_cmd_buffer(cmd1, cmd2);
}

void vulkan::VulkanRHI::submit(rhi::QueueType queueType, bool waitAfterSubmitting)
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

bool vulkan::VulkanRHI::present()
{
	if (!_device->get_graphics_queue()->present(_swapChain.get(), _currentImageIndex))
	{
		recreate_swap_chain();
		return false;
	}
	return true;
}

void vulkan::VulkanRHI::wait_fences()
{
	_cmdManager->wait_fences();
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

void vulkan::VulkanRHI::copy_buffer_to_texture(rhi::CommandBuffer* cmd, rhi::Buffer* srcBuffer, rhi::Texture* dstTexture)
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
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, nullptr, 0, nullptr,
		1, &imageBarrierToReadable);
}

void vulkan::VulkanRHI::set_viewports(rhi::CommandBuffer* cmd, std::vector<rhi::Viewport>& viewports)
{
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

void vulkan::VulkanRHI::set_scissors(rhi::CommandBuffer* cmd, std::vector<rhi::Scissor>& scissors)
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

void vulkan::VulkanRHI::push_constants(rhi::CommandBuffer* cmd, rhi::Pipeline* pipeline, void* data)
{
	VulkanPipeline* vkPipeline = get_vk_obj(pipeline);
	vkPipeline->push_constants(get_vk_obj(cmd)->get_handle(), data);
}

void vulkan::VulkanRHI::bind_vertex_buffer(rhi::CommandBuffer* cmd, rhi::Buffer* buffer)
{
	if (!cmd || !buffer)
	{
		LOG_ERROR("VulkanRHI::bind_vertex_buffer(): Invalid pointers")
		return;
	}
	if (!has_flag(buffer->bufferInfo.bufferUsage, rhi::ResourceUsage::VERTEX_BUFFER))
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
	if (!has_flag(buffer->bufferInfo.bufferUsage, rhi::ResourceUsage::INDEX_BUFFER))
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
	vkPipeline->bind(vkCmd->get_handle(), _currentImageIndex);
}

void vulkan::VulkanRHI::begin_render_pass(rhi::CommandBuffer* cmd, rhi::RenderPass* renderPass, rhi::ClearValues& clearValues)
{
	if (!cmd || !renderPass)
	{
		LOG_ERROR("VulkanRHI::bind_render_pass(): Invalid pointers")
		return;
	}

	VulkanCommandBuffer* vkCmd = get_vk_obj(cmd);
	VulkanRenderPass* pass = get_vk_obj(renderPass);

	VkRenderPassBeginInfo beginInfo = pass->get_begin_info(clearValues, _currentImageIndex);

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

void vulkan::VulkanRHI::begin_rendering(rhi::CommandBuffer* cmd, rhi::RenderingBeginInfo* beginInfo)
{
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
	VulkanTexture* vulkanTexture = get_vk_obj(beginInfo->renderTargets[0].target->texture);
	VkExtent3D extent3D = vulkanTexture->get_extent();
	renderingInfo.renderArea.extent = { extent3D.width, extent3D.height };
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

	renderingInfo.colorAttachmentCount = colorAttachments.size();
	renderingInfo.pColorAttachments = colorAttachments.data();
	vkCmdBeginRendering(get_vk_obj(cmd)->get_handle(), &renderingInfo);
}

void vulkan::VulkanRHI::end_rendering(rhi::CommandBuffer* cmd)
{
	vkCmdEndRendering(get_vk_obj(cmd)->get_handle());
}

void vulkan::VulkanRHI::begin_rendering_swap_chain(rhi::CommandBuffer* cmd, rhi::ClearValues* clearValues)
{
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.pDepthAttachment = nullptr;
	renderingInfo.pStencilAttachment = nullptr;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.layerCount = 1;
	renderingInfo.renderArea.offset.x = 0;
	renderingInfo.renderArea.offset.y = 0;
	renderingInfo.renderArea.extent.width = _swapChain->get_width();
	renderingInfo.renderArea.extent.height = _swapChain->get_height();
	
	VkRenderingAttachmentInfo attachmentInfo{};
	attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentInfo.imageView = _swapChain->get_image_view_handle(_currentImageIndex);
	attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentInfo.clearValue.color = { { clearValues->color[0], clearValues->color[1], clearValues->color[2], clearValues->color[3] } };

	renderingInfo.pColorAttachments = &attachmentInfo;
	set_swap_chain_image_barrier(cmd, false);
	vkCmdBeginRendering(get_vk_obj(cmd)->get_handle(), &renderingInfo);
}

void vulkan::VulkanRHI::end_rendering_swap_chain(rhi::CommandBuffer* cmd)
{
	vkCmdEndRendering(get_vk_obj(cmd)->get_handle());
	set_swap_chain_image_barrier(cmd, true);
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

void vulkan::VulkanRHI::wait_for_gpu()
{
	VK_CHECK(vkDeviceWaitIdle(_device->get_device()));
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
	allocatorInfo.physicalDevice = _device->get_physical_device();
	allocatorInfo.device = _device->get_device();
	allocatorInfo.instance = _instance;
	vmaCreateAllocator(&allocatorInfo, &_allocator);
	LOG_INFO("End creating allocator")
}

void vulkan::VulkanRHI::create_pipeline_cache()
{
	size_t size = 0;
	void* data = nullptr;
	
	if (io::Utils::exists(_fileSystem->get_project_root_path(), "intermediate/pipeline_cache.bin"))
		data = _fileSystem->map_to_read(_fileSystem->get_project_root_path() + "/intermediate/pipeline_cache.bin", size);

	VkPipelineCacheCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	createInfo.initialDataSize = size;
	createInfo.pInitialData = data;

	VK_CHECK(vkCreatePipelineCache(_device.get()->get_device(), &createInfo, nullptr, &_pipelineCache));

	if (data)
		_fileSystem->unmap_after_reading(data);
}

void vulkan::VulkanRHI::save_pipeline_cache()
{
	VkDevice device = _device.get()->get_device();
	size_t cacheSize;
	vkGetPipelineCacheData(device, _pipelineCache, &cacheSize, nullptr);
	std::vector<uint8_t> cacheData(cacheSize);
	vkGetPipelineCacheData(device, _pipelineCache, &cacheSize, cacheData.data());

	io::URI cachePath = _fileSystem->get_project_root_path() + "/intermediate/pipeline_cache.bin";
	io::Stream* stream = _fileSystem->open(cachePath, "wb");
	stream->write(cacheData.data(), sizeof(uint8_t), cacheSize);
	_fileSystem->close(stream);
}

void vulkan::VulkanRHI::set_swap_chain_image_barrier(rhi::CommandBuffer* cmd, bool useAfterDrawingImageBarrier)
{
	VkImageMemoryBarrier2 imageBarrier{};
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	imageBarrier.image = _swapChain->get_image_handle(_currentImageIndex);
	if (useAfterDrawingImageBarrier)
	{
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		imageBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_NONE;
	}
	else
	{
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		imageBarrier.srcAccessMask = VK_ACCESS_2_NONE;
		imageBarrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
	}
	imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
	imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
	imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	imageBarrier.subresourceRange.baseArrayLayer = 0;
	imageBarrier.subresourceRange.baseMipLevel = 0;
	imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	
	VkDependencyInfo dependencyInfo{};
	dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependencyInfo.memoryBarrierCount = 0;
	dependencyInfo.pMemoryBarriers = nullptr;
	dependencyInfo.pBufferMemoryBarriers = nullptr;
	dependencyInfo.bufferMemoryBarrierCount = 0;
	dependencyInfo.imageMemoryBarrierCount = 1;
	dependencyInfo.pImageMemoryBarriers = &imageBarrier;

	vkCmdPipelineBarrier2(get_vk_obj(cmd)->get_handle(), &dependencyInfo);
}

void vulkan::VulkanRHI::recreate_swap_chain()
{
	vkDeviceWaitIdle(_device->get_device());
	uint32_t width = _mainWindow->get_width();
	uint32_t height = _mainWindow->get_height();
	for (auto& pair : _viewIndicesByTexturePtr)
	{
		VulkanTexture* vulkanTexture = pair.first;
		vulkanTexture->destroy_texture(_allocator);
		VkImageCreateInfo& imageCreateInfo = pair.second->imageCreateInfo;
		imageCreateInfo.extent = { width, height, 1 };
		vulkanTexture->create_texture(imageCreateInfo, &_allocator, VMA_MEMORY_USAGE_GPU_ONLY);

		for (auto& imageViewDesc : pair.second->viewDescriptions)
		{
			VulkanTextureView* vulkanTextureView = _vulkanTextureViews[imageViewDesc.viewArrayIndex].get();
			vulkanTextureView->destroy(_device.get());
			VkImageViewCreateInfo& viewCreateInfo = imageViewDesc.imageViewCreateInfo;
			viewCreateInfo.image = vulkanTexture->get_handle();
			vulkanTextureView->create(_device.get(), viewCreateInfo);
			_descriptorManager->allocate_bindless_descriptor(vulkanTextureView, TextureDescriptorHeapType::TEXTURES);
		}
	}
	_swapChain->recreate(width, height);
}
