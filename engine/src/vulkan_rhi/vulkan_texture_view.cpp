#include "vulkan_texture_view.h"
#include "vulkan_common.h"

using namespace ad_astris::vulkan;

VulkanTextureView::VulkanTextureView(VulkanDevice* device, VkImageViewCreateInfo& info)
{
	VK_CHECK(vkCreateImageView(device->get_device(), &info, nullptr, &_imageView));
}

VulkanTextureView::VulkanTextureView(
	VulkanDevice* device,
	rhi::TextureViewInfo* textureViewInfo,
	rhi::Texture* texture,
	VkImageViewCreateInfo& outCreateInfo)
{
	create(device, textureViewInfo, texture, outCreateInfo);
}

void VulkanTextureView::create(VulkanDevice* device, VkImageViewCreateInfo& info)
{
	VK_CHECK(vkCreateImageView(device->get_device(), &info, nullptr, &_imageView));
}

void VulkanTextureView::create(
	VulkanDevice* device,
	rhi::TextureViewInfo* textureViewInfo,
	rhi::Texture* texture,
	VkImageViewCreateInfo& outCreateInfo)
{
	parse_texture_view_info(textureViewInfo, texture, outCreateInfo);
	create(device, outCreateInfo);
}

void VulkanTextureView::destroy(VulkanDevice* device)
{
	if (_imageView != VK_NULL_HANDLE)
		vkDestroyImageView(device->get_device(), _imageView, nullptr);
	_imageView = VK_NULL_HANDLE;
}

void VulkanTextureView::parse_texture_view_info(
	rhi::TextureViewInfo* viewInfo,
	rhi::Texture* texture,
	VkImageViewCreateInfo& outCreateInfo)
{
	rhi::TextureInfo textureInfo = texture->textureInfo;
	VulkanTexture* vkTexture = static_cast<VulkanTexture*>(texture->data);
	
	VkFormat format = get_format(textureInfo.format);
	VkImageUsageFlags imgUsage = get_image_usage(textureInfo.textureUsage);

	VkImageAspectFlags aspectFlags;
	if (viewInfo->textureAspect == rhi::TextureAspect::UNDEFINED)
	{
		if ((imgUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		else
			aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	else
	{
		aspectFlags = get_image_aspect(viewInfo->textureAspect);
	}
	
	outCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

	if (textureInfo.textureDimension == rhi::TextureDimension::TEXTURE1D)
	{
		if (textureInfo.layersCount == 1)
		{
			outCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
		}
		else
		{
			outCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		}
	}
	else if (textureInfo.textureDimension == rhi::TextureDimension::TEXTURE2D)
	{
		if (textureInfo.layersCount > 1)
		{
			if (has_flag(textureInfo.resourceFlags, rhi::ResourceFlags::CUBE_TEXTURE))
			{
				if (textureInfo.layersCount > 6)
				{
					outCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
				}
				else
				{
					outCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
				}
			}
			else
			{
				outCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			}
		}
		else
		{
			outCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		}
	}
	else if (textureInfo.textureDimension == rhi::TextureDimension::TEXTURE3D)
	{
		outCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
	}
	else
	{
		LOG_ERROR("VulkanRHI::create_texture_view(): Undefined texture dimension")
		return;
	}
	
	//createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	outCreateInfo.image = vkTexture->get_handle();
	outCreateInfo.format = format;
	outCreateInfo.subresourceRange.baseMipLevel = viewInfo->baseMipLevel;
	outCreateInfo.subresourceRange.levelCount = textureInfo.mipLevels;
	outCreateInfo.subresourceRange.baseArrayLayer = viewInfo->baseLayer;
	outCreateInfo.subresourceRange.layerCount = textureInfo.layersCount;
	outCreateInfo.subresourceRange.aspectMask = aspectFlags;
}
