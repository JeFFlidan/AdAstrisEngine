#include "vulkan_buffer.h"
#include "vulkan_common.h"
#include "profiler/logger.h"

using namespace ad_astris::vulkan;

VulkanBuffer::VulkanBuffer(VulkanDevice* device, rhi::BufferInfo* bufferInfo)
{
	create_buffer(device, bufferInfo);
}

void VulkanBuffer::create_buffer(VulkanDevice* device, rhi::BufferInfo* bufferInfo)
{
	VkBufferCreateInfo bufferCreateInfo{};
	VmaAllocationCreateInfo allocCreateInfo{};
	
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = bufferInfo->size;
	bufferCreateInfo.usage = get_buffer_usage(bufferInfo->bufferUsage);
	
	if (!bufferCreateInfo.usage)
		LOG_FATAL("VulkanBuffer::parse_buffer_info(): Invalid buffer usage")
	
	allocCreateInfo.usage = get_memory_usage(bufferInfo->memoryUsage);

	switch (bufferInfo->memoryUsage)
	{
		case rhi::MemoryUsage::CPU:
		case rhi::MemoryUsage::CPU_TO_GPU:
			allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			break;
		case rhi::MemoryUsage::GPU_TO_CPU:
			allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			break;
		default:
			allocCreateInfo.flags = 0;
			break;
	}

	_bufferSize = bufferInfo->size;
	
	if (_buffer != VK_NULL_HANDLE)
		destroy(device);
	
	VK_CHECK(vmaCreateBuffer(device->get_allocator(), &bufferCreateInfo, &allocCreateInfo, &_buffer, &_allocation, nullptr));
}

void VulkanBuffer::destroy(VulkanDevice* device)
{
	if (_buffer != VK_NULL_HANDLE)
	{
		vmaDestroyBuffer(device->get_allocator(), _buffer, _allocation);
		_buffer = VK_NULL_HANDLE;
	}
}
