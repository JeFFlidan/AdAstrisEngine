#pragma once
#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

namespace ad_astris::vulkan
{
	class VulkanTexture
	{
		public:
			VkImage _image = VK_NULL_HANDLE;
			VmaAllocation _allocation;
			VkExtent3D _extent;
			uint32_t _mipLevels;

			VulkanTexture() = default;
			VulkanTexture(VkImageCreateInfo info, VmaAllocator* allocator, VmaMemoryUsage memoryUsage);
			
			void create_texture(VkImageCreateInfo info, VmaAllocator* allocator, VmaMemoryUsage memoryUsage);
		
		private:
			void allocate_texture(VkImageCreateInfo info, VmaAllocator* allocator, VmaMemoryUsage memoryUsage);
	};
}
