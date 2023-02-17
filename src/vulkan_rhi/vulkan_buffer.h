#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace ad_astris::vulkan
{
	class VulkanBuffer
	{
		public:
			VkBuffer buffer = VK_NULL_HANDLE;
			VmaAllocation allocation;

			uint64_t bufferSize = 0;

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
		
		protected:
			void allocate_buffer(VmaAllocator* allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);		
	};
}