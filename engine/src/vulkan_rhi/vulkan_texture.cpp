#include "vulkan_texture.h"
#include "vulkan_device.h"
#include "vulkan_common.h"
#include "profiler/logger.h"

using namespace ad_astris::vulkan;

VulkanTexture::VulkanTexture(VulkanDevice* device, VkImageCreateInfo& info, VmaMemoryUsage memoryUsage)
{
	allocate_texture(info, device->get_allocator(), memoryUsage);
}

VulkanTexture::VulkanTexture(VulkanDevice* device, rhi::TextureInfo* textureInfo, VkImageCreateInfo& outCreateInfo)
{
	create_texture(device, textureInfo, outCreateInfo);
}

void VulkanTexture::destroy(VulkanDevice* device)
{
	if (_image != VK_NULL_HANDLE)
	{
		vmaDestroyImage(device->get_allocator(), _image, _allocation);
		_image = VK_NULL_HANDLE;
	}
}

void VulkanTexture::create_texture(VulkanDevice* device, VkImageCreateInfo& info, VmaMemoryUsage memoryUsage)
{
	allocate_texture(info, device->get_allocator(), memoryUsage);
}

void VulkanTexture::create_texture(VulkanDevice* device, rhi::TextureInfo* textureInfo, VkImageCreateInfo& outCreateInfo)
{
	VmaAllocationCreateInfo allocCreateInfo{};
	parse_texture_info(textureInfo, outCreateInfo, allocCreateInfo);
	allocate_texture(device, outCreateInfo, allocCreateInfo);
}

void VulkanTexture::parse_texture_info(rhi::TextureInfo* inTextureInfo, VkImageCreateInfo& outImageInfo, VmaAllocationCreateInfo& outAllocInfo)
{
	if (inTextureInfo->format == rhi::Format::UNDEFINED)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined format")
		return;
	}
	if (inTextureInfo->textureUsage == rhi::ResourceUsage::UNDEFINED)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined texture usage.")
		return;
	}
	if (inTextureInfo->memoryUsage == rhi::MemoryUsage::UNDEFINED)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined memory usage.")
		return;
	}
	if (inTextureInfo->samplesCount == rhi::SampleCount::UNDEFINED)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined sample count.")
		return;
	}
	if (inTextureInfo->textureDimension == rhi::TextureDimension::UNDEFINED)
	{
		LOG_ERROR("VulkanRHI::create_texture(): Undefined texture dimension.")
		return;
	}

	outImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	outImageInfo.pNext = nullptr;
	outImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	outImageInfo.format = get_format(inTextureInfo->format);
	outImageInfo.arrayLayers = inTextureInfo->layersCount;
	outImageInfo.mipLevels = inTextureInfo->mipLevels;
	outImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	outImageInfo.extent = VkExtent3D{ inTextureInfo->width, inTextureInfo->height, 1 };
	outImageInfo.samples = get_sample_count(inTextureInfo->samplesCount);
	
	if (has_flag(inTextureInfo->resourceFlags, rhi::ResourceFlags::CUBE_TEXTURE))
	{
		outImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	VkImageUsageFlags imgUsage = get_image_usage(inTextureInfo->textureUsage);
	outImageInfo.usage = imgUsage;
	outImageInfo.imageType = get_image_type(inTextureInfo->textureDimension);

	outAllocInfo.usage = get_memory_usage(inTextureInfo->memoryUsage);
	outAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void VulkanTexture::allocate_texture(VkImageCreateInfo& info, VmaAllocator allocator, VmaMemoryUsage memoryUsage)
{
	this->_extent = info.extent;
	this->_mipLevels = info.mipLevels;

	VmaAllocationCreateInfo imgAllocInfo{};
	imgAllocInfo.usage = memoryUsage;
	imgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);	// VMA allocate the image into VRAM in all situations

	vmaCreateImage(allocator, &info, &imgAllocInfo, &_image, &_allocation, nullptr);
}

void VulkanTexture::allocate_texture(VulkanDevice* device, VkImageCreateInfo& imageCreateInfo, VmaAllocationCreateInfo& allocInfo)
{
	_extent = imageCreateInfo.extent;
	_mipLevels = imageCreateInfo.mipLevels;
	vmaCreateImage(device->get_allocator(), &imageCreateInfo, &allocInfo, &_image, &_allocation, nullptr);
}
