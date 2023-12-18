#include "vulkan_buffer.h"
#include "vulkan_common.h"
#include "profiler/logger.h"

using namespace ad_astris::vulkan;

VulkanBuffer::VulkanBuffer(VulkanDevice* device, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	create_buffer(device, allocSize, usage, memoryUsage);
}

VulkanBuffer::VulkanBuffer(VulkanDevice* device, rhi::BufferInfo* bufferInfo)
{
	create_buffer(device, bufferInfo);
}

void VulkanBuffer::create_buffer(VulkanDevice* device, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	VkBufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = allocSize;
	createInfo.usage = usage;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = memoryUsage;
	
	allocate_buffer(device, createInfo, allocInfo);
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
	memcpy(_mappedData, srcBuffer, sizeInBytes);
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
	if (_buffer != VK_NULL_HANDLE)
	{
		if (_mappedData)
			vmaUnmapMemory(device->get_allocator(), _allocation);
		
		vmaDestroyBuffer(device->get_allocator(), _buffer, _allocation);
		_buffer = VK_NULL_HANDLE;
	}
}

void VulkanBuffer::parse_buffer_info(rhi::BufferInfo* inBufferInfo, VkBufferCreateInfo& outCreateInfo, VmaAllocationCreateInfo& outAllocationInfo)
{
	outCreateInfo.pNext = nullptr;
	outCreateInfo.flags = 0;
	outCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	outCreateInfo.size = inBufferInfo->size;
	outCreateInfo.usage = get_buffer_usage(inBufferInfo->bufferUsage);
	
	if (!outCreateInfo.usage)
		LOG_FATAL("VulkanBuffer::parse_buffer_info(): Invalid buffer usage")

	outAllocationInfo.usage = get_memory_usage(inBufferInfo->memoryUsage);
	if (outAllocationInfo.usage == VMA_MEMORY_USAGE_UNKNOWN)
		LOG_FATAL("VulkanBuffer::parse_buffer_info(): Invalid memory usage")

	_bufferSize = inBufferInfo->size;
}

void VulkanBuffer::allocate_buffer(VulkanDevice* device, VkBufferCreateInfo& createInfo, VmaAllocationCreateInfo& allocInfo)
{
	if (_buffer != VK_NULL_HANDLE)
		destroy(device);
	
	VK_CHECK(vmaCreateBuffer(device->get_allocator(), &createInfo, &allocInfo, &_buffer, &_allocation, nullptr));
	if (allocInfo.usage == VMA_MEMORY_USAGE_CPU_ONLY || allocInfo.usage == VMA_MEMORY_USAGE_CPU_TO_GPU)
		VK_CHECK(vmaMapMemory(device->get_allocator(), _allocation, &_mappedData));
}
