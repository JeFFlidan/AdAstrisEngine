#include "vulkan_buffer.h"
#include "profiler/logger.h"

using namespace ad_astris;

vulkan::VulkanBuffer::VulkanBuffer(VmaAllocator* allocator, uint64_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	allocate_buffer(allocator, allocSize, usage, memoryUsage);
}

void vulkan::VulkanBuffer::create_buffer(VmaAllocator* allocator, uint64_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	allocate_buffer(allocator, allocSize, usage, memoryUsage);
}

void vulkan::VulkanBuffer::copy_from(VmaAllocator* allocator, void* srcBuffer, uint64_t sizeInBytes)
{
	void* data;
	vmaMapMemory(*allocator, _allocation, &data);
	memcpy(data, srcBuffer, sizeInBytes);
	vmaUnmapMemory(*allocator, _allocation);
	_bufferSize = sizeInBytes;
}

VkDescriptorBufferInfo vulkan::VulkanBuffer::get_info(bool isStorage, VkDeviceSize offset)
{
	VkDescriptorBufferInfo info;
	info.buffer = _buffer;
	info.offset = offset;
	info.range = (isStorage) ? VK_WHOLE_SIZE : _bufferSize;
	return info;
}

void vulkan::VulkanBuffer::copy_buffer_cmd(
	VkCommandBuffer cmd ,
	VulkanBuffer* srcBuffer,
	VulkanBuffer* dstBuffer,
	VkDeviceSize dstOffset,
	VkDeviceSize srcOffset)
{
	dstBuffer->_bufferSize = srcBuffer->_bufferSize;
	VkBufferCopy copy;
	copy.dstOffset = dstOffset;
	copy.srcOffset = srcOffset;
	copy.size = srcBuffer->_bufferSize;
	vkCmdCopyBuffer(cmd, srcBuffer->_buffer, dstBuffer->_buffer, 1, &copy);
	
}

void vulkan::VulkanBuffer::destroy_buffer(VmaAllocator* allocator)
{
    vmaDestroyBuffer(*allocator, _buffer, _allocation);
}

void vulkan::VulkanBuffer::allocate_buffer(
	VmaAllocator* allocator,
	uint64_t allocSize,
	VkBufferUsageFlags usage,
	VmaMemoryUsage memoryUsage)
{
	_bufferSize = allocSize;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;

	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo{};
	vmaallocInfo.usage = memoryUsage;

    VkResult res = vmaCreateBuffer(*allocator, &bufferInfo, &vmaallocInfo, &_buffer, &_allocation, nullptr);
    
    if (res != VK_SUCCESS)
		LOG_ERROR("Failed to allocate buffer")
}