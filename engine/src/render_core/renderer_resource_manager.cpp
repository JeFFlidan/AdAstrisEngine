#include "renderer_resource_manager.h"

using namespace ad_astris;
using namespace rcore;
using namespace impl;

void RendererResourceManager::init(RendererResourceManagerInitContext& initContext)
{
	assert(initContext.rhi != nullptr);
	_rhi = initContext.rhi;
	_bufferPool.allocate_new_pool(64);
}

void RendererResourceManager::cleanup_staging_buffers()
{
	for (auto& buffer : _stagingBuffers)
		_rhi->destroy_buffer(&buffer);
	_stagingBuffers.clear();
	_isDeviceWaiting.store(false);
}

void RendererResourceManager::allocate_gpu_buffer(const std::string& bufferName, uint64_t size, rhi::ResourceUsage bufferUsage)
{
	auto it = _bufferByItsName.find(bufferName);
	if (it != _bufferByItsName.end())
	{
		LOG_WARNING("RendererResourceManager::allocate_gpu_buffer(): Buffer with name has been already created.", bufferName)
		return;
	}
	
	rhi::BufferInfo bufferInfo;
	bufferInfo.size = size;
	bufferInfo.bufferUsage = bufferUsage;
	bufferInfo.memoryUsage = rhi::MemoryUsage::GPU;
	rhi::Buffer* buffer = _bufferPool.allocate();
	_rhi->create_buffer(buffer, &bufferInfo);

	std::scoped_lock<std::mutex> locker(_gpuBufferMutex);
	_bufferByItsName[bufferName] = buffer;
}

void RendererResourceManager::allocate_vertex_buffer(const std::string& bufferName, uint64_t size)
{
	allocate_gpu_buffer(bufferName, size, rhi::ResourceUsage::VERTEX_BUFFER | rhi::ResourceUsage::TRANSFER_DST);
}

void RendererResourceManager::allocate_index_buffer(const std::string& bufferName, uint64_t size)
{
	allocate_gpu_buffer(bufferName, size, rhi::ResourceUsage::INDEX_BUFFER | rhi::ResourceUsage::TRANSFER_DST);
}

void RendererResourceManager::allocate_indirect_buffer(const std::string& bufferName, uint64_t size)
{
	allocate_gpu_buffer(bufferName, size, rhi::ResourceUsage::INDIRECT_BUFFER | rhi::ResourceUsage::TRANSFER_DST | rhi::ResourceUsage::STORAGE_BUFFER);
}

void RendererResourceManager::allocate_storage_buffer(const std::string& bufferName, uint64_t size)
{
	allocate_gpu_buffer(bufferName, size, rhi::ResourceUsage::STORAGE_BUFFER | rhi::ResourceUsage::TRANSFER_DST);
}

bool RendererResourceManager::update_buffer(
	rhi::CommandBuffer* cmd,
	const std::string& bufferName,
	uint64_t objectSizeInBytes,
	void* allObjects,
	uint64_t allObjectCount,
	uint64_t newObjectCount)
{
	rhi::Buffer* gpuBuffer = const_cast<rhi::Buffer*>(get_buffer(bufferName));
	uint64_t offsetInBytes = (allObjectCount - newObjectCount) * objectSizeInBytes;
	uint64_t newObjectsSizeInBytes = newObjectCount * objectSizeInBytes;

	if (offsetInBytes + newObjectsSizeInBytes < gpuBuffer->size)
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
	_rhi->create_buffer(gpuBuffer, &gpuBuffer->bufferInfo);
	rhi::Buffer& stagingBuffer = get_new_staging_buffer();
	allocate_staging_buffer(stagingBuffer, allObjects, 0, allObjectCount * objectSizeInBytes);
	_rhi->copy_buffer(cmd, &stagingBuffer, gpuBuffer);
	
	return false;
}

const rhi::Buffer* RendererResourceManager::get_buffer(const std::string& bufferName)
{
	auto it = _bufferByItsName.find(bufferName);
	if (it == _bufferByItsName.end())
		LOG_FATAL("RenderResourceManager::get_buffer(): No buffer with name {}", bufferName)

	return it->second;
}

void RendererResourceManager::allocate_staging_buffer(rhi::Buffer& buffer, void* allObjects, uint64_t offset, uint64_t newObjectsSize)
{
	rhi::BufferInfo bufferInfo;
	bufferInfo.size = newObjectsSize;
	bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC | rhi::ResourceUsage::STORAGE_BUFFER;
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
