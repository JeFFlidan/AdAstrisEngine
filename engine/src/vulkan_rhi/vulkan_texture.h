#pragma once

#include "vulkan_descriptor.h"
#include "vulkan_object.h"
#include "rhi/resources.h"
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace ad_astris::vulkan
{
	class VulkanDevice;
	
	class VulkanTexture : public IVulkanObject
	{
		public:
			VulkanTexture() = default;
			VulkanTexture(VulkanDevice* device, VkImageCreateInfo& info, VmaMemoryUsage memoryUsage);
			VulkanTexture(VulkanDevice* device, rhi::TextureInfo* textureInfo, VkImageCreateInfo& outCreateInfo);
		
			void create_texture(VulkanDevice* device, VkImageCreateInfo& info, VmaMemoryUsage memoryUsage);
			void create_texture(VulkanDevice* device, rhi::TextureInfo* textureInfo, VkImageCreateInfo& outCreateInfo);
			void destroy(VulkanDevice* device) override;

			VkImage get_handle() const { return _image; }
			void set_handle(VkImage image) { _image = image; }
			VkExtent3D get_extent() const { return _extent; }
			void* get_mapped_data() const { return _mappedData; }
		
		private:
			VkImage _image = VK_NULL_HANDLE;
			VmaAllocation _allocation;
			VkExtent3D _extent;
			uint32_t _mipLevels;
			void* _mappedData;

			void parse_texture_info(rhi::TextureInfo* inTextureInfo, VkImageCreateInfo& outImageInfo, VmaAllocationCreateInfo& outAllocInfo);
			void allocate_texture(VulkanDevice* device, VkImageCreateInfo& imageCreateInfo, VmaAllocationCreateInfo& allocCreateInfo);
	};
}
