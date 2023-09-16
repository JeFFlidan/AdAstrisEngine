#pragma once

#include "api.h"
#include "vulkan_descriptor.h"
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace ad_astris::vulkan
{
	class VulkanDevice;
	
	class VulkanTexture
	{
		public:
			VulkanTexture() = default;
			VulkanTexture(VkImageCreateInfo info, VmaAllocator* allocator, VmaMemoryUsage memoryUsage);

			void destroy_texture();
			
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

	class VulkanTextureView : public VulkanDescriptor
	{
		public:
			VulkanTextureView() = default;
			VulkanTextureView(VulkanDevice* device, VkImageViewCreateInfo& info);
			void destroy(VulkanDevice* device);
			
			VkImageView get_handle() { return _imageView; }
			void set_handle(VkImageView imageView) { _imageView = imageView; }
		
		private:
			VkImageView _imageView{ VK_NULL_HANDLE };
	};

	class VulkanSampler : public VulkanDescriptor
	{
		public:
			VulkanSampler(VulkanDevice* device, VkSamplerCreateInfo& info);
			void destroy(VulkanDevice* device);

			VkSampler get_handle() { return _sampler; }

		private:
			VkSampler _sampler{ VK_NULL_HANDLE };
	};
}
