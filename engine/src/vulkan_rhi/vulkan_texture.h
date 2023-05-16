﻿#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

namespace ad_astris::vulkan
{
	class VulkanTexture
	{
		public:
			VulkanTexture() = default;
			VulkanTexture(VkImageCreateInfo info, VmaAllocator* allocator, VmaMemoryUsage memoryUsage);
			~VulkanTexture();
			
			void create_texture(VkImageCreateInfo info, VmaAllocator* allocator, VmaMemoryUsage memoryUsage);

			VkImage get_handle() { return _image; }
			VkExtent3D get_extent() { return _extent; }
		
		private:
			VmaAllocator* _allocator;
			VkImage _image = VK_NULL_HANDLE;
			VmaAllocation _allocation;
			VkExtent3D _extent;
			uint32_t _mipLevels;
		
			void allocate_texture(VkImageCreateInfo info, VmaAllocator* allocator, VmaMemoryUsage memoryUsage);
	};
}