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
			uint8_t _mipLevels;

			VulkanTexture(
				VmaAllocator* allocator,
				VmaMemoryUsage memoryUsage,
				VkExtent3D extent,
				uint8_t mipLevels,
				uint8_t layersCount,
				VkFormat format,
				VkImageUsageFlags imageUsage,
				VkSampleCountFlagBits samplesCount,
				VkImageType imageType);
			
			void create_texture(
				VmaAllocator* allocator,
				VmaMemoryUsage memoryUsage,
				VkExtent3D extent,
				uint8_t mipLevels,
				uint8_t layersCount,
				VkFormat format,
				VkImageUsageFlags imageUsage,
				VkSampleCountFlagBits samplesCount,
				VkImageType imageType);
		
		private:
			void allocate_texture(
				VmaAllocator* allocator,
				VmaMemoryUsage memoryUsage,
				VkExtent3D extent,
				uint8_t mipLevels,
				uint8_t layersCount,
				VkFormat format,
				VkImageUsageFlags imageUsage,
				VkSampleCountFlagBits samplesCount,
				VkImageType imageType);
	};
}
