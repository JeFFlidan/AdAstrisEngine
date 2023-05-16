#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace ad_astris::vulkan
{
	class VulkanBuffer
	{
		public:
			VulkanBuffer() = default;
			VulkanBuffer(VmaAllocator* allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
			void create_buffer(VmaAllocator* allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
					
			void copy_from(VmaAllocator* allocator, void* srcBuffer, size_t sizeInBytes);
			void destroy_buffer(VmaAllocator* allocator);
			VkDescriptorBufferInfo get_info(bool isStorage = false, VkDeviceSize offset = 0);
			static void copy_buffer_cmd(
				VkCommandBuffer cmd,
				VulkanBuffer* srcBuffer,
				VulkanBuffer* dstBuffer,
				VkDeviceSize dstOffset = 0, 
				VkDeviceSize srcOffset = 0);

			VkBuffer* get_handle() { return &_buffer; }
			uint64_t get_size() { return _bufferSize; }
			
		private:
			VkBuffer _buffer = VK_NULL_HANDLE;
			VmaAllocation _allocation;

			uint64_t _bufferSize = 0;
			void allocate_buffer(VmaAllocator* allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);		
	};
}