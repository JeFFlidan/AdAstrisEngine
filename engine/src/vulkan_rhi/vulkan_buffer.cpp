#include "vulkan_buffer.h"
#include "vulkan_common.h"
#include "profiler/logger.h"

using namespace ad_astris::vulkan;

VulkanBuffer::VulkanBuffer(VulkanDevice* device, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	allocate_buffer(device->get_allocator(), allocSize, usage, memoryUsage);
}

VulkanBuffer::VulkanBuffer(VulkanDevice* device, rhi::BufferInfo* bufferInfo)
{
	create_buffer(device, bufferInfo);
}

void VulkanBuffer::create_buffer(VulkanDevice* device, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	allocate_buffer(device->get_allocator(), allocSize, usage, memoryUsage);
}

void VulkanBuffer::create_buffer(VulkanDevice* device, rhi::BufferInfo* bufferInfo)
{
	VkBufferCreateInfo bufferCreateInfo{};
	VmaAllocationCreateInfo allocCreateInfo{};
	parse_buffer_info(bufferInfo, bufferCreateInfo, allocCreateInfo);
	allocate_buffer(device, bufferCreateInfo, allocCreateInfo);
}

void VulkanBuffer::copy_from(VulkanDevice* device, void* srcBuffer, size_t sizeInBytes)
{
	void* data;
	vmaMapMemory(device->get_allocator(), _allocation, &data);
	memcpy(data, srcBuffer, sizeInBytes);
	vmaUnmapMemory(device->get_allocator(), _allocation);
	_bufferSize = sizeInBytes;
}

VkDescriptorBufferInfo VulkanBuffer::get_info(bool isStorage, VkDeviceSize offset)
{
	VkDescriptorBufferInfo info;
	info.buffer = _buffer;
	info.offset = offset;
	info.range = (isStorage) ? VK_WHOLE_SIZE : _bufferSize;
	return info;
}

void VulkanBuffer::copy_buffer_cmd(
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

void VulkanBuffer::destroy(VulkanDevice* device)
{
    vmaDestroyBuffer(device->get_allocator(), _buffer, _allocation);
}

void VulkanBuffer::parse_buffer_info(rhi::BufferInfo* inBufferInfo, VkBufferCreateInfo& outCreateInfo, VmaAllocationCreateInfo& outAllocationInfo)
{
	outCreateInfo.pNext = nullptr;
	outCreateInfo.flags = 0;
	outCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	outCreateInfo.size = inBufferInfo->size;
	outCreateInfo.usage = get_buffer_usage(inBufferInfo->bufferUsage);
	
	if (!outCreateInfo.usage)
	{
		LOG_FATAL("VulkanBuffer::parse_buffer_info(): Invalid buffer usage")
		return;
	}

	outAllocationInfo.usage = get_memory_usage(inBufferInfo->memoryUsage);
	if (outAllocationInfo.usage == VMA_MEMORY_USAGE_UNKNOWN)
	{
		LOG_FATAL("Invalid memory usage (buffer)")
	}
}

void VulkanBuffer::allocate_buffer(
	VmaAllocator allocator,
	size_t allocSize,
	VkBufferUsageFlags usage,
	VmaMemoryUsage memoryUsage)
{
	if (_buffer != VK_NULL_HANDLE)
		vmaDestroyBuffer(allocator, _buffer, _allocation);
	
	_bufferSize = allocSize;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;

	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo{};
	vmaallocInfo.usage = memoryUsage;

    VkResult res = vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &_buffer, &_allocation, nullptr);
    
    if (res != VK_SUCCESS)
		LOG_ERROR("Failed to allocate buffer")
}

void VulkanBuffer::allocate_buffer(VulkanDevice* device, VkBufferCreateInfo& createInfo, VmaAllocationCreateInfo& allocInfo)
{
	VK_CHECK(vmaCreateBuffer(device->get_allocator(), &createInfo, &allocInfo, &_buffer, &_allocation, nullptr));
}
