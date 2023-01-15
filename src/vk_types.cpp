#include "vk_types.h"
#include "vk_engine.h"
#include "logger.h"
#include <vulkan/vulkan_core.h>

AllocatedBuffer::AllocatedBuffer(VulkanEngine* engine, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	allocate_buffer(engine, allocSize, usage, memoryUsage);
}

void AllocatedBuffer::create_buffer(VulkanEngine* engine, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	allocate_buffer(engine, allocSize, usage, memoryUsage);
}

void AllocatedBuffer::copy_from(VulkanEngine* engine, void* srcBuffer, size_t sizeInBytes)
{
	void* data;
	vmaMapMemory(engine->_allocator, _allocation, &data);
	memcpy(data, srcBuffer, sizeInBytes);
	vmaUnmapMemory(engine->_allocator, _allocation);
	_bufferSize = sizeInBytes;
}

VkDescriptorBufferInfo AllocatedBuffer::get_info(bool isStorage, VkDeviceSize offset)
{
	VkDescriptorBufferInfo info;
	info.buffer = _buffer;
	info.offset = offset;
	info.range = (isStorage) ? VK_WHOLE_SIZE : _bufferSize;
	return info;
}

void AllocatedBuffer::copy_buffer_cmd(VulkanEngine* engine, VkCommandBuffer cmd , AllocatedBuffer* srcBuffer, AllocatedBuffer* dstBuffer, VkDeviceSize dstOffset, VkDeviceSize srcOffset)
{
	dstBuffer->_bufferSize = srcBuffer->_bufferSize;
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

void AllocatedBuffer::allocate_buffer(VulkanEngine* engine, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	_bufferSize = allocSize;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;

	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo{};
	vmaallocInfo.usage = memoryUsage;

    VkResult res = vmaCreateBuffer(engine->_allocator, &bufferInfo, &vmaallocInfo, &_buffer, &_allocation, nullptr);
    
    if (res != VK_SUCCESS)
		LOG_ERROR("Failed to allocate buffer");
}

void Texture::destroy_texture(VulkanEngine* engine)
{
	vkDestroyImageView(engine->_device, imageView, nullptr);
	vmaDestroyImage(engine->_allocator, imageData.image, imageData.allocation);
}

void Attachment::destroy_attachment(VulkanEngine* engine)
{
	destroy_texture(engine);
}

