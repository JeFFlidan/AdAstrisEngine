#pragma once

#include "vulkan_descriptor.h"
#include "vulkan_object.h"
#include "vulkan_api.h"
#include "rhi/resources.h"

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
			uint64_t get_size() const { return _bufferSize; }
			void* get_mapped_data() const { return _mappedData; }
			
		private:
			VkBuffer _buffer = VK_NULL_HANDLE;
			VmaAllocation _allocation;
			uint64_t _bufferSize = 0;
			void* _mappedData{ nullptr };
		
			void parse_buffer_info(rhi::BufferInfo* inBufferInfo, VkBufferCreateInfo& outCreateInfo, VmaAllocationCreateInfo& outAllocationInfo);
			void allocate_buffer(VulkanDevice* device, VkBufferCreateInfo& createInfo, VmaAllocationCreateInfo& allocInfo);
	};
}