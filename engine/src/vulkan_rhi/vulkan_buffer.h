#pragma once

#include "vulkan_descriptor.h"
#include "vulkan_object.h"
#include "rhi/resources.h"
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace ad_astris::vulkan
{
	class VulkanDevice;
	
	class VulkanBuffer : public VulkanDescriptor, public IVulkanObject
	{
		public:
			VulkanBuffer() = default;
			VulkanBuffer(VulkanDevice* device, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
			VulkanBuffer(VulkanDevice* device, rhi::BufferInfo* bufferInfo);
			void create_buffer(VulkanDevice* device, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
			void create_buffer(VulkanDevice* device, rhi::BufferInfo* bufferInfo);
			void destroy(VulkanDevice* device) override;
					
			void copy_from(VulkanDevice* device, void* srcBuffer, size_t sizeInBytes);
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
		
			void parse_buffer_info(rhi::BufferInfo* inBufferInfo, VkBufferCreateInfo& outCreateInfo, VmaAllocationCreateInfo& outAllocationInfo);
			void allocate_buffer(VmaAllocator allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
			void allocate_buffer(VulkanDevice* device, VkBufferCreateInfo& createInfo, VmaAllocationCreateInfo& allocInfo);
	};
}