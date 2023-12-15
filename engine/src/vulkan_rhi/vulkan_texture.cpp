﻿#include "vulkan_texture.h"
#include "vulkan_device.h"
#include "vulkan_common.h"
#include "profiler/logger.h"

using namespace ad_astris::vulkan;

VulkanTexture::VulkanTexture(VulkanDevice* device, VkImageCreateInfo& info, VmaMemoryUsage memoryUsage)
{
	create_texture(device, info, memoryUsage);
}

VulkanTexture::VulkanTexture(VulkanDevice* device, rhi::TextureInfo* textureInfo, VkImageCreateInfo& outCreateInfo)
{
	create_texture(device, textureInfo, outCreateInfo);
}

void VulkanTexture::destroy(VulkanDevice* device)
{
	if (_image != VK_NULL_HANDLE)
	{
		if (_mappedData)
			vmaUnmapMemory(device->get_allocator(), _allocation);
		
		vmaDestroyImage(device->get_allocator(), _image, _allocation);
		_image = VK_NULL_HANDLE;
	}
}

void VulkanTexture::create_texture(VulkanDevice* device, VkImageCreateInfo& info, VmaMemoryUsage memoryUsage)
{
	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = memoryUsage;
	allocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	allocate_texture(device, info, allocInfo);
}

void VulkanTexture::create_texture(VulkanDevice* device, rhi::TextureInfo* textureInfo, VkImageCreateInfo& outCreateInfo)
{
	VmaAllocationCreateInfo allocInfo{};
	parse_texture_info(textureInfo, outCreateInfo, allocInfo);
	allocate_texture(device, outCreateInfo, allocInfo);
}

void VulkanTexture::parse_texture_info(rhi::TextureInfo* inTextureInfo, VkImageCreateInfo& outImageInfo, VmaAllocationCreateInfo& outAllocInfo)
{
	if (inTextureInfo->format == rhi::Format::UNDEFINED)
		LOG_FATAL("VulkanTexture::parse_texture_info(): Undefined format")
	
	if (inTextureInfo->textureUsage == rhi::ResourceUsage::UNDEFINED)
		LOG_FATAL("VulkanTexture::parse_texture_info(): Undefined texture usage.")
	
	if (inTextureInfo->memoryUsage == rhi::MemoryUsage::UNDEFINED)
		LOG_FATAL("VulkanTexture::parse_texture_info(): Undefined memory usage.")
	
	if (inTextureInfo->samplesCount == rhi::SampleCount::UNDEFINED)
		LOG_FATAL("VulkanTexture::parse_texture_info(): Undefined sample count.")
	
	if (inTextureInfo->textureDimension == rhi::TextureDimension::UNDEFINED)
		LOG_FATAL("VulkanTexture::parse_texture_info(): Undefined texture dimension.")

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

void VulkanTexture::allocate_texture(VulkanDevice* device, VkImageCreateInfo& imageCreateInfo, VmaAllocationCreateInfo& allocInfo)
{
	if (_image == VK_NULL_HANDLE)
		destroy(device);
	
	_extent = imageCreateInfo.extent;
	_mipLevels = imageCreateInfo.mipLevels;
	VK_CHECK(vmaCreateImage(device->get_allocator(), &imageCreateInfo, &allocInfo, &_image, &_allocation, nullptr));
	if (allocInfo.usage == VMA_MEMORY_USAGE_CPU_ONLY || allocInfo.usage == VMA_MEMORY_USAGE_CPU_TO_GPU)
		VK_CHECK(vmaMapMemory(device->get_allocator(), _allocation, &_mappedData));
}
