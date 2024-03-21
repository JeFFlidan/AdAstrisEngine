#pragma once

#include "vulkan_api.h"
#include "vulkan_object.h"
#include "rhi/resources.h"

namespace ad_astris::vulkan
{
	class VulkanDevice;
	
	class VulkanTexture : public IVulkanObject
	{
		public:
			VulkanTexture() = default;
			VulkanTexture(VulkanDevice* device, rhi::TextureInfo* textureInfo, VkImageCreateInfo& outCreateInfo);
		
			void create_texture(VulkanDevice* device, rhi::TextureInfo* textureInfo, VkImageCreateInfo& outCreateInfo);
			void create_texture(VulkanDevice* device, const VkImageCreateInfo& imageCreateInfo);
			void destroy(VulkanDevice* device) override;

			VkImage get_handle() const { return _image; }
			VkExtent3D get_extent() const { return _extent; }
			VmaAllocation get_allocation() const { return _allocation; }
		
		private:
			VkImage _image = VK_NULL_HANDLE;
			VmaAllocation _allocation;
			VkExtent3D _extent{ 1, 1, 1};
			uint32_t _mipLevels{ 0 };
	};
}
