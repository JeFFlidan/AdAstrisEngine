#pragma once

#include "vulkan_api.h"
#include "vulkan_descriptor.h"
#include "vulkan_object.h"
#include "rhi/resources.h"

namespace ad_astris::vulkan
{
	class VulkanTextureView : public VulkanDescriptor, public IVulkanObject
	{
		public:
			VulkanTextureView() = default;
			VulkanTextureView(VulkanDevice* device, VkImageViewCreateInfo& info);
			VulkanTextureView(
				VulkanDevice* device,
				rhi::TextureViewInfo* textureViewInfo,
				rhi::Texture* texture,
				VkImageViewCreateInfo& outCreateInfo);
		
			void create(VulkanDevice* device, VkImageViewCreateInfo& info);
			void create(
				VulkanDevice* device,
				rhi::TextureViewInfo* textureViewInfo,
				rhi::Texture* texture,
				VkImageViewCreateInfo& outCreateInfo);
			void destroy(VulkanDevice* device) override;
				
			VkImageView get_handle() { return _imageView; }
			void set_handle(VkImageView imageView) { _imageView = imageView; }
		
		private:
			VkImageView _imageView{ VK_NULL_HANDLE };

			void parse_texture_view_info(rhi::TextureViewInfo* viewInfo, rhi::Texture* texture, VkImageViewCreateInfo& outCreateInfo);
	};
}