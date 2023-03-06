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
	vmaMapMemory(*allocator, allocation, &data);
	memcpy(data, srcBuffer, sizeInBytes);
	vmaUnmapMemory(*allocator, allocation);
	bufferSize = sizeInBytes;
}

VkDescriptorBufferInfo vulkan::VulkanBuffer::get_info(bool isStorage, VkDeviceSize offset)
{
	VkDescriptorBufferInfo info;
	info.buffer = buffer;
	info.offset = offset;
	info.range = (isStorage) ? VK_WHOLE_SIZE : bufferSize;
	return info;
}

void vulkan::VulkanBuffer::copy_buffer_cmd(
	VkCommandBuffer cmd ,
	VulkanBuffer* srcBuffer,
	VulkanBuffer* dstBuffer,
	VkDeviceSize dstOffset,
	VkDeviceSize srcOffset)
{
	dstBuffer->bufferSize = srcBuffer->bufferSize;
	VkBufferCopy copy;
	copy.dstOffset = dstOffset;
	copy.srcOffset = srcOffset;
	copy.size = srcBuffer->bufferSize;
	vkCmdCopyBuffer(cmd, srcBuffer->buffer, dstBuffer->buffer, 1, &copy);
	
}

void vulkan::VulkanBuffer::destroy_buffer(VmaAllocator* allocator)
{
    vmaDestroyBuffer(*allocator, buffer, allocation);
}

void vulkan::VulkanBuffer::allocate_buffer(
	VmaAllocator* allocator,
	uint64_t allocSize,
	VkBufferUsageFlags usage,
	VmaMemoryUsage memoryUsage)
{
	bufferSize = allocSize;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;

	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo{};
	vmaallocInfo.usage = memoryUsage;

    VkResult res = vmaCreateBuffer(*allocator, &bufferInfo, &vmaallocInfo, &buffer, &allocation, nullptr);
    
    if (res != VK_SUCCESS)
		LOG_ERROR("Failed to allocate buffer")
}