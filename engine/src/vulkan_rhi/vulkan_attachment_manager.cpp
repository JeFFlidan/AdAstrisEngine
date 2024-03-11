#include "vulkan_attachment_manager.h"

#include "vulkan_device.h"

using namespace ad_astris::vulkan;

VulkanAttachmentManager::VulkanAttachmentManager()
{
	_attachmentDescPool.allocate_new_pool(64);
}

VulkanAttachmentManager::~VulkanAttachmentManager()
{
	_attachmentDescPool.cleanup();
}

void VulkanAttachmentManager::add_attachment_texture(VulkanTexture* texture, const VkImageCreateInfo& createInfo)
{
	assert(texture);
	if (is_attachment(createInfo.usage))
	{
		_attachDescByTexturePtr[texture] = _attachmentDescPool.allocate();
		_attachDescByTexturePtr[texture]->imageCreateInfo = createInfo;
	}
}

void VulkanAttachmentManager::add_attachment_texture_view(
	VulkanTextureView* textureView,
	VulkanTexture* texture,
	VkImageUsageFlags imageUsage,
	const VkImageViewCreateInfo& createInfo)
{
	assert(texture, textureView);
	if (is_attachment(imageUsage))
	{
		AttachmentDesc::ViewDesc& viewDesc = _attachDescByTexturePtr[texture]->viewDescriptions.emplace_back();
		viewDesc.textureView = textureView;
		viewDesc.imageViewCreateInfo = createInfo;
	}
}

void VulkanAttachmentManager::recreate_attachments(
	VulkanDevice* device,
	VulkanDescriptorManager* descriptorManager,
	uint32_t width,
	uint32_t height)
{
	assert(device, descriptorManager);
	for (auto& pair : _attachDescByTexturePtr)
	{
		VulkanTexture* texture = pair.first;
		texture->destroy(device);
		VkImageCreateInfo& imageCreateInfo = pair.second->imageCreateInfo;
		imageCreateInfo.extent = { width, height, 1 };
		texture->create_texture(device, imageCreateInfo);

		for (auto& imageViewDesc : pair.second->viewDescriptions)
		{
			VulkanTextureView* textureView = imageViewDesc.textureView;
			textureView->destroy(device);
			VkImageViewCreateInfo& viewCreateInfo = imageViewDesc.imageViewCreateInfo;
			viewCreateInfo.image = texture->get_handle();
			textureView->create(device, viewCreateInfo);
			// TODO fix
			//descriptorManager->allocate_bindless_descriptor(textureView, TextureDescriptorHeapType::TEXTURES);
		}
	}
}

bool VulkanAttachmentManager::is_attachment(VkImageUsageFlags imageUsage)
{
	return (imageUsage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		|| (imageUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
}
