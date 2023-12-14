#pragma once

#include "vulkan_texture.h"
#include "vulkan_texture_view.h"
#include "vulkan_descriptor_manager.h"
#include "core/pool_allocator.h"

namespace ad_astris::vulkan
{
	class VulkanAttachmentManager
	{
		public:
			VulkanAttachmentManager();
			~VulkanAttachmentManager();
		
			void add_attachment_texture(VulkanTexture* texture, const VkImageCreateInfo& createInfo);
			void add_attachment_texture_view(
				VulkanTextureView* textureView,
				VulkanTexture* texture,
				VkImageUsageFlags imageUsage,
				const VkImageViewCreateInfo& createInfo);
			void recreate_attachments(VulkanDevice* device, VulkanDescriptorManager* descriptorManager, uint32_t width, uint32_t height);

		private:
			struct AttachmentDesc
			{
				VkImageCreateInfo imageCreateInfo;
				struct ViewDesc
				{
					VulkanTextureView* textureView;
					VkImageViewCreateInfo imageViewCreateInfo;
				};
				std::vector<ViewDesc> viewDescriptions;
			};
			ThreadSafePoolAllocator<AttachmentDesc> _attachmentDescPool;
			std::unordered_map<VulkanTexture*, AttachmentDesc*> _attachDescByTexturePtr;

			bool is_attachment(VkImageUsageFlags imageUsage);
	};
}