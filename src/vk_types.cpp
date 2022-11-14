#include "vk_types.h"
#include "vk_engine.h"
#include "logger.h"
#include <vulkan/vulkan_core.h>

AllocatedBuffer AllocatedBuffer::create_buffer(VulkanEngine* engine, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	AllocatedBuffer newBuffer;
	newBuffer._bufferSize = allocSize;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;

	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo{};
	vmaallocInfo.usage = memoryUsage;

    VkResult res = vmaCreateBuffer(engine->_allocator, &bufferInfo, &vmaallocInfo, &newBuffer._buffer, &newBuffer._allocation, nullptr);

    if (res != VK_SUCCESS)
    {
		LOG_ERROR("Failed to allocate buffer");
		return AllocatedBuffer();
    }
	
	return newBuffer;
}

void AllocatedBuffer::copy_from(VulkanEngine* engine, void* srcBuffer, size_t sizeInBytes)
{
	void* data;
	vmaMapMemory(engine->_allocator, _allocation, &data);
	memcpy(data, srcBuffer, sizeInBytes);
	vmaUnmapMemory(engine->_allocator, _allocation);
}

void AllocatedBuffer::copyBufferCmd(VulkanEngine* engine, VkCommandBuffer cmd , AllocatedBuffer* srcBuffer, AllocatedBuffer* dstBuffer, VkDeviceSize dstOffset, VkDeviceSize srcOffset)
{
	VkBufferCopy copy;
	copy.dstOffset = dstOffset;
	copy.srcOffset = srcOffset;
	copy.size = srcBuffer->_bufferSize;
	vkCmdCopyBuffer(cmd, srcBuffer->_buffer, dstBuffer->_buffer, 1, &copy);
}

void AllocatedBuffer::destroy_buffer(VulkanEngine* engine)
{
    vmaDestroyBuffer(engine->_allocator, _buffer, _allocation);
}

