#include "renderer_resource_manager.h"

using namespace ad_astris;
using namespace rcore;
using namespace impl;

void RendererResourceManager::init(RendererResourceManagerInitContext& initContext)
{
	assert(initContext.rhi != nullptr);
	_rhi = initContext.rhi;
	_bufferPool.allocate_new_pool(64);
	_texturePool.allocate_new_pool(128);
	_textureViewPool.allocate_new_pool(256);
}

void RendererResourceManager::cleanup()
{
	_bufferPool.cleanup();
	_texturePool.cleanup();
	_textureViewPool.cleanup();
}

void RendererResourceManager::cleanup_staging_buffers()
{
	for (auto& buffer : _stagingBuffers)
		_rhi->destroy_buffer(&buffer);
	_stagingBuffers.clear();
	_isDeviceWaiting.store(false);
}

rhi::Buffer* RendererResourceManager::allocate_buffer(rhi::BufferInfo& bufferInfo)
{
	rhi::Buffer* buffer = _bufferPool.allocate();
	_rhi->create_buffer(buffer, &bufferInfo);
	return buffer;
}

rhi::Buffer* RendererResourceManager::allocate_buffer(const std::string& bufferName, rhi::BufferInfo& bufferInfo)
{
	rhi::Buffer* buffer = check_buffer(bufferName);
	if (buffer)
		return buffer;

	buffer = _bufferPool.allocate();
	_rhi->create_buffer(buffer, &bufferInfo);

	std::scoped_lock<std::mutex> locker(_gpuBufferMutex);
	_bufferByItsName[bufferName] = buffer;
	return buffer;
}

rhi::Buffer* RendererResourceManager::allocate_gpu_buffer(const std::string& bufferName, uint64_t size, rhi::ResourceUsage bufferUsage)
{
	rhi::Buffer* buffer = check_buffer(bufferName);
	if (buffer)
		return buffer;
	
	buffer = allocate_gpu_buffer(size, bufferUsage);

	std::scoped_lock<std::mutex> locker(_gpuBufferMutex);
	_bufferByItsName[bufferName] = buffer;
	return buffer;
}

rhi::Buffer* RendererResourceManager::allocate_vertex_buffer(const std::string& bufferName, uint64_t size)
{
	return allocate_gpu_buffer(bufferName, size, rhi::ResourceUsage::VERTEX_BUFFER | rhi::ResourceUsage::TRANSFER_DST);
}

rhi::Buffer* RendererResourceManager::allocate_index_buffer(const std::string& bufferName, uint64_t size)
{
	return allocate_gpu_buffer(bufferName, size, rhi::ResourceUsage::INDEX_BUFFER | rhi::ResourceUsage::TRANSFER_DST);
}

rhi::Buffer* RendererResourceManager::allocate_indirect_buffer(uint64_t size)
{
	return allocate_gpu_buffer(size, rhi::ResourceUsage::INDIRECT_BUFFER | rhi::ResourceUsage::TRANSFER_DST | rhi::ResourceUsage::STORAGE_BUFFER);
}

rhi::Buffer* RendererResourceManager::allocate_indirect_buffer(const std::string& bufferName, uint64_t size)
{
	return allocate_gpu_buffer(bufferName, size, rhi::ResourceUsage::INDIRECT_BUFFER | rhi::ResourceUsage::TRANSFER_DST | rhi::ResourceUsage::STORAGE_BUFFER);
}

rhi::Buffer* RendererResourceManager::allocate_storage_buffer(uint64_t size)
{
	return allocate_gpu_buffer(size, rhi::ResourceUsage::STORAGE_BUFFER | rhi::ResourceUsage::TRANSFER_DST);
}

rhi::Buffer* RendererResourceManager::allocate_storage_buffer(const std::string& bufferName, uint64_t size)
{
	return allocate_gpu_buffer(bufferName, size, rhi::ResourceUsage::STORAGE_BUFFER | rhi::ResourceUsage::TRANSFER_DST);
}

void RendererResourceManager::reallocate_buffer(rhi::Buffer* buffer, uint64_t newSize)
{
	rhi::BufferInfo& bufferInfo = buffer->bufferInfo;
	switch (bufferInfo.memoryUsage)
	{
		case rhi::MemoryUsage::CPU:
		case rhi::MemoryUsage::CPU_TO_GPU:
		{
			std::vector<uint8_t> tempData(bufferInfo.size);
			memcpy(tempData.data(), buffer->mappedData, bufferInfo.size);
			_rhi->destroy_buffer(buffer);
			bufferInfo.size = newSize;
			_rhi->create_buffer(buffer);
			memcpy(buffer->mappedData, tempData.data(), tempData.size());
			break;
		}
		case rhi::MemoryUsage::AUTO:
		case rhi::MemoryUsage::GPU:
			LOG_ERROR("RendererResourceManager::reallocate_buffer(): You can only reallocate buffers that have memoryUsage = CPU or memoryUsage = CPU_TO_GPU")
	}
}

rhi::Buffer* RendererResourceManager::reallocate_buffer(const std::string& bufferName, uint64_t newSize)
{
	rhi::Buffer* buffer = get_buffer(bufferName);
	reallocate_buffer(buffer, newSize);
	return buffer;
}

bool RendererResourceManager::update_buffer(
	rhi::CommandBuffer* cmd,
	const std::string& bufferName,
	uint64_t objectSizeInBytes,
	void* allObjects,
	uint64_t allObjectCount,
	uint64_t newObjectCount)
{
	rhi::Buffer* gpuBuffer = get_buffer(bufferName);
	uint64_t offsetInBytes = (allObjectCount - newObjectCount) * objectSizeInBytes;
	uint64_t newObjectsSizeInBytes = newObjectCount * objectSizeInBytes;

	if (offsetInBytes + newObjectsSizeInBytes <= gpuBuffer->bufferInfo.size)
	{
		rhi::Buffer& stagingBuffer = get_new_staging_buffer();
		allocate_staging_buffer(stagingBuffer, allObjects, offsetInBytes, newObjectsSizeInBytes);
		_rhi->copy_buffer(cmd, &stagingBuffer, gpuBuffer, newObjectsSizeInBytes, 0, offsetInBytes);
		return true;
	}

	if (!_isDeviceWaiting.load())
	{
		_rhi->wait_for_gpu();
		_isDeviceWaiting.store(true);
	}
	_rhi->destroy_buffer(gpuBuffer);
	gpuBuffer->bufferInfo.size *= 2.0f;			// TODO Make it more effective
	_rhi->create_buffer(gpuBuffer, &gpuBuffer->bufferInfo);
	rhi::Buffer& stagingBuffer = get_new_staging_buffer();
	allocate_staging_buffer(stagingBuffer, allObjects, 0, allObjectCount * objectSizeInBytes);
	_rhi->copy_buffer(cmd, &stagingBuffer, gpuBuffer);
	
	return false;
}

bool RendererResourceManager::update_buffer(
	rhi::CommandBuffer* cmd,
	const std::string& srcBufferName, 
	const std::string& dstBufferName,
	uint64_t objectSizeInBytes,
	uint64_t allObjectCount,
	uint64_t newObjectCount)
{
	rhi::Buffer* srcBuffer = get_buffer(srcBufferName);
	rhi::Buffer* dstBuffer = get_buffer(dstBufferName);
	return update_buffer(cmd, srcBuffer, dstBuffer, objectSizeInBytes, allObjectCount, newObjectCount);
}

bool RendererResourceManager::update_buffer(
	rhi::CommandBuffer* cmd,
	rhi::Buffer* srcBuffer,
	rhi::Buffer* dstBuffer,
	uint64_t objectSizeInBytes,
	uint64_t allObjectCount,
	uint64_t newObjectCount)
{
	uint64_t offsetInBytes = (allObjectCount - newObjectCount) * objectSizeInBytes;
	uint64_t newObjectsSizeInBytes = newObjectCount * objectSizeInBytes;
	
	if (offsetInBytes + newObjectsSizeInBytes <= dstBuffer->bufferInfo.size)
	{
		_rhi->copy_buffer(cmd, srcBuffer, dstBuffer, newObjectsSizeInBytes, 0, offsetInBytes);
		return true;
	}

	if (!_isDeviceWaiting.load())
	{
		_rhi->wait_for_gpu();
		_isDeviceWaiting.store(true);
	}
	_rhi->destroy_buffer(dstBuffer);
	dstBuffer->bufferInfo.size *= 2;
	_rhi->create_buffer(dstBuffer);
	_rhi->copy_buffer(cmd, srcBuffer, dstBuffer);
	return false;
}

rhi::Buffer* RendererResourceManager::get_buffer(const std::string& bufferName)
{
	std::scoped_lock<std::mutex> locker(_gpuBufferMutex);
	auto it = _bufferByItsName.find(bufferName);
	if (it == _bufferByItsName.end())
		LOG_FATAL("RenderResourceManager::get_buffer(): No buffer with name {}", bufferName)

	return it->second;
}

void RendererResourceManager::add_buffer(const std::string& bufferName, rhi::Buffer& buffer)
{
	// TODO
}

void RendererResourceManager::bind_buffer_to_name(const std::string& bufferName, rhi::Buffer* buffer)
{
	_bufferByItsName[bufferName] = buffer;
}

rhi::Texture* RendererResourceManager::allocate_texture(const std::string& textureName, rhi::TextureInfo& textureInfo)
{
	rhi::Texture* texture = check_texture(textureName);
	if (texture)
		return texture;

	texture = _texturePool.allocate();
	_rhi->create_texture(texture, &textureInfo);

	std::scoped_lock<std::mutex> locker(_textureMutex);
	_textureByItsName[textureName] = texture;
	return texture;
}

rhi::Texture* RendererResourceManager::allocate_gpu_texture(
	const std::string& textureName,
	uint64_t width,
	uint64_t height,
	rhi::Format format,
	rhi::ResourceUsage usage,
	rhi::ResourceFlags flags,
	uint32_t mipLevels,
	uint32_t layersCount)
{
	rhi::Texture* texture = check_texture(textureName);
	if (texture)
		return texture;
	
	texture = _texturePool.allocate();
	rhi::TextureInfo& textureInfo = texture->textureInfo;
	textureInfo.width = width;
	textureInfo.height = height;
	textureInfo.layersCount = layersCount;
	textureInfo.mipLevels = mipLevels;
	textureInfo.memoryUsage = rhi::MemoryUsage::GPU;
	textureInfo.resourceFlags = flags;
	textureInfo.textureDimension = rhi::TextureDimension::TEXTURE2D;
	textureInfo.textureUsage = usage;
	textureInfo.samplesCount = rhi::SampleCount::BIT_1;
	textureInfo.format = format;
	_rhi->create_texture(texture);

	std::scoped_lock<std::mutex> locker(_textureMutex);
	_textureByItsName[textureName] = texture;
	return texture;
}

rhi::Texture* RendererResourceManager::allocate_color_attachment(
	const std::string& textureName,
	uint64_t width,
	uint64_t height,
	rhi::ResourceFlags flags,
	uint32_t mipLevels,
	uint32_t layersCount)
{
	return allocate_gpu_texture(
		textureName,
		width,
		height,
		rhi::Format::R8G8B8A8_UNORM,
		rhi::ResourceUsage::SAMPLED_TEXTURE | rhi::ResourceUsage::COLOR_ATTACHMENT | rhi::ResourceUsage::TRANSFER_SRC,
		flags,
		mipLevels,
		layersCount);
}

rhi::Texture* RendererResourceManager::allocate_depth_stencil_attachment(
	const std::string& textureName,
	uint64_t width,
	uint64_t height,
	rhi::ResourceFlags flags,
	uint32_t mipLevels,
	uint32_t layersCount)
{
	return allocate_gpu_texture(
		textureName,
		width,
		height,
		rhi::Format::D32_SFLOAT,
		rhi::ResourceUsage::SAMPLED_TEXTURE | rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT | rhi::ResourceUsage::TRANSFER_SRC,
		flags,
		mipLevels,
		layersCount);
}

rhi::Texture* RendererResourceManager::allocate_cubemap(
	const std::string& textureName,
	uint64_t width,
	uint64_t height,
	rhi::Format format,
	rhi::ResourceUsage usage,
	uint32_t mipLevels,
	rhi::ResourceFlags flags)
{
	return allocate_gpu_texture(textureName, width, height, format, usage, flags, mipLevels, 6);
}

rhi::Texture* RendererResourceManager::allocate_custom_texture(
	const std::string& textureName,
	uint64_t width,
	uint64_t height,
	rhi::ResourceFlags flags,
	uint32_t mipLevels,
	uint32_t layersCount)
{
	return allocate_gpu_texture(
		textureName,
		width,
		height,
		rhi::Format::R8G8B8A8_UNORM,
		rhi::ResourceUsage::TRANSFER_DST | rhi::ResourceUsage::SAMPLED_TEXTURE | rhi::ResourceUsage::TRANSFER_SRC,
		flags,
		mipLevels,
		layersCount);
}

rhi::TextureView* RendererResourceManager::allocate_texture_view(
	const std::string& textureViewName,
	const std::string& textureName,
	rhi::TextureViewInfo& info)
{
	rhi::TextureView* textureView = check_texture_view(textureViewName);
	if (textureView)
		return textureView;

	rhi::Texture* texture;
	{
		std::scoped_lock<std::mutex> _locker(_textureMutex);
		auto it = _textureByItsName.find(textureName);
		if (it == _textureByItsName.end())
			LOG_FATAL("RendererResourceManager::allocate_texture_view(): Can't create texture view for the texture {}", textureName)
		texture = it->second;
	}

	textureView = _textureViewPool.allocate();
	_rhi->create_texture_view(textureView, &info, texture);

	std::scoped_lock<std::mutex> locker(_textureViewMutex);
	_textureViewByItsName[textureViewName] = textureView;
	return textureView;
}

rhi::TextureView* RendererResourceManager::allocate_texture_view(
	const std::string& textureViewName,
	const std::string& textureName,
	uint32_t baseMipLevel,
	uint32_t mipLevels,
	uint32_t baseLayer,
	uint32_t layerCount,
	rhi::TextureAspect aspect)
{
	rhi::TextureView* textureView = check_texture_view(textureViewName);
	if (textureView)
		return textureView;
	
	rhi::Texture* texture;
	{
		std::scoped_lock<std::mutex> _locker(_textureMutex);
		auto it = _textureByItsName.find(textureName);
		if (it == _textureByItsName.end())
			LOG_FATAL("RendererResourceManager::allocate_texture_view(): Can't create texture view for the texture {}", textureName)
		texture = it->second;
	}

	textureView = _textureViewPool.allocate();
	textureView->viewInfo.baseLayer = baseLayer;
	textureView->viewInfo.baseMipLevel = baseMipLevel;
	textureView->viewInfo.textureAspect = aspect;
	textureView->viewInfo.mipLevels = mipLevels ? mipLevels : texture->textureInfo.mipLevels;
	textureView->viewInfo.layerCount = layerCount ? layerCount : texture->textureInfo.layersCount;

	_rhi->create_texture_view(textureView, texture);

	std::scoped_lock<std::mutex> locker(_textureViewMutex);
	_textureViewByItsName[textureViewName] = textureView;
	return textureView;
}

void RendererResourceManager::update_2d_texture(rhi::CommandBuffer* cmd, const std::string& textureName, void* textureData, uint32_t width, uint32_t height)
{
	rhi::Texture* texture = get_texture(textureName);

	rhi::Buffer buffer;
	allocate_staging_buffer(buffer, textureData, 0, width * height * 4);

	_rhi->copy_buffer_to_texture(cmd, &buffer, texture);
}

void RendererResourceManager::generate_mipmaps(rhi::CommandBuffer* cmd, const std::string& textureName)
{
	generate_mipmaps(cmd, get_texture(textureName));
}

void RendererResourceManager::generate_mipmaps(rhi::CommandBuffer* cmd, rhi::Texture* texture)
{
	rhi::PipelineBarrier barrier(
		texture,
		rhi::ResourceLayout::SHADER_READ,
		rhi::ResourceLayout::TRANSFER_DST,
		0);
	_rhi->add_pipeline_barriers(cmd, { barrier });
	
	const rhi::TextureInfo& textureInfo = texture->textureInfo;
	uint32_t width = textureInfo.width;
	uint32_t height = textureInfo.height;
	for (uint32_t mipLevel = 0; mipLevel != textureInfo.mipLevels; ++mipLevel)
	{
		uint32_t halfWidth = width / 2;
		uint32_t halfHeight = height / 2;

		barrier.set_texture_barrier(
			texture,
			rhi::ResourceLayout::TRANSFER_DST,
			rhi::ResourceLayout::TRANSFER_SRC,
			mipLevel,
			1);
		_rhi->add_pipeline_barriers(cmd, { barrier });

		if (mipLevel < textureInfo.mipLevels - 1)
		{
			_rhi->blit_texture(
				cmd,
				texture,
				texture,
				{ (int32_t)width, (int32_t)height, 1 },
				{ (int32_t)halfWidth, (int32_t)halfHeight, 1},
				mipLevel,
				mipLevel + 1);
		}

		width = halfWidth;
		height = halfHeight;
	}

	barrier.set_texture_barrier(
		texture,
		rhi::ResourceLayout::TRANSFER_SRC,
		rhi::ResourceLayout::SHADER_READ,
		0);
	_rhi->add_pipeline_barriers(cmd, { barrier });
}

rhi::Texture* RendererResourceManager::get_texture(const std::string& textureName)
{
	std::scoped_lock<std::mutex> locker(_textureMutex);
	auto it = _textureByItsName.find(textureName);
	if (it == _textureByItsName.end())
		LOG_FATAL("RenderResourceManager::get_texture(): No texture with name {}", textureName)

	return it->second;
}

rhi::TextureView* RendererResourceManager::get_texture_view(const std::string& textureViewName)
{
	std::scoped_lock<std::mutex> locker(_textureViewMutex);
    auto it = _textureViewByItsName.find(textureViewName);
    if (it == _textureViewByItsName.end())
    	LOG_FATAL("RenderResourceManager::get_texture_view(): No texture view with name {}", textureViewName)

    return it->second;
}

void RendererResourceManager::add_texture(const std::string& textureName, rhi::Texture& texture)
{
	// TODO
}

void RendererResourceManager::add_texture_view(const std::string& textureViewName, rhi::TextureView& textureView)
{
	// TODO
}

rhi::Buffer* RendererResourceManager::allocate_gpu_buffer(uint64_t size, rhi::ResourceUsage bufferUsage)
{
	rhi::BufferInfo bufferInfo;
	bufferInfo.size = size;
	bufferInfo.bufferUsage = bufferUsage;
	bufferInfo.memoryUsage = rhi::MemoryUsage::GPU;
	rhi::Buffer* buffer = _bufferPool.allocate();
	_rhi->create_buffer(buffer, &bufferInfo);
	return buffer;
}

void RendererResourceManager::allocate_staging_buffer(rhi::Buffer& buffer, void* allObjects, uint64_t offset, uint64_t newObjectsSize)
{
	rhi::BufferInfo bufferInfo;
	bufferInfo.size = newObjectsSize;
	bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC;
	bufferInfo.memoryUsage = rhi::MemoryUsage::CPU;
	
	uint8_t* data = reinterpret_cast<uint8_t*>(allObjects);
	data += offset;
	
	_rhi->create_buffer(&buffer, &bufferInfo, data);
}

rhi::Buffer& RendererResourceManager::get_new_staging_buffer()
{
	std::scoped_lock<std::mutex> locker(_stagingBufferMutex);
	return _stagingBuffers.emplace_back();
}

rhi::Buffer* RendererResourceManager::check_buffer(const std::string& bufferName)
{
	std::scoped_lock<std::mutex> locker(_gpuBufferMutex);
	auto it = _bufferByItsName.find(bufferName);
	if (it != _bufferByItsName.end())
	{
		LOG_WARNING("RendererResourceManager::allocate_gpu_buffer(): Buffer with name {} has been already created.", bufferName)
		return it->second;
	}
	return nullptr;
}

rhi::Texture* RendererResourceManager::check_texture(const std::string& textureName)
{
	std::scoped_lock<std::mutex> locker(_textureMutex);
	auto it = _textureByItsName.find(textureName);
	if (it != _textureByItsName.end())
	{
		LOG_WARNING("RendererResourceManager::allocate_gpu_texture(): Texture with name {} has been already created.", textureName)
		return it->second;
	}
	return nullptr;
}

rhi::TextureView* RendererResourceManager::check_texture_view(const std::string& textureViewName)
{
	std::scoped_lock<std::mutex> locker(_textureViewMutex);
	auto it = _textureViewByItsName.find(textureViewName);
	if (it != _textureViewByItsName.end())
	{
		LOG_WARNING("RendererResourceManager::allocate_gpu_texture(): Texture view with name {} has been already created.", textureViewName)
		return it->second;
	}
	return nullptr;
}
