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
			VulkanBuffer(VulkanDevice* device, rhi::BufferInfo* bufferInfo);
			void create_buffer(VulkanDevice* device, rhi::BufferInfo* bufferInfo);
			void destroy(VulkanDevice* device) override;

			VkBuffer get_handle() const { return _buffer; }
			uint64_t get_size() const { return _bufferSize; }
			VmaAllocation get_allocation() const { return _allocation; }
			
		private:
			VkBuffer _buffer = VK_NULL_HANDLE;
			VmaAllocation _allocation;
			uint64_t _bufferSize = 0;
	};
}