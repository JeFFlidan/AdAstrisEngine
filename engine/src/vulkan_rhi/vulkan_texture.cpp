#include "vulkan_texture.h"
#include "vulkan_device.h"
#include "vulkan_common.h"
#include "profiler/logger.h"
#include "rhi/utils.h"

using namespace ad_astris::vulkan;

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

void VulkanTexture::create_texture(VulkanDevice* device, rhi::TextureInfo* textureInfo, VkImageCreateInfo& outCreateInfo)
{
	if (textureInfo->format == rhi::Format::UNDEFINED)
		LOG_FATAL("VulkanTexture::parse_texture_info(): Undefined format")
	
	if (textureInfo->textureUsage == rhi::ResourceUsage::UNDEFINED)
		LOG_FATAL("VulkanTexture::parse_texture_info(): Undefined texture usage.")
	
	if (textureInfo->samplesCount == rhi::SampleCount::UNDEFINED)
		LOG_FATAL("VulkanTexture::parse_texture_info(): Undefined sample count.")
	
	if (textureInfo->textureDimension == rhi::TextureDimension::UNDEFINED)
		LOG_FATAL("VulkanTexture::parse_texture_info(): Undefined texture dimension.")

	outCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	outCreateInfo.pNext = nullptr;
	outCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	outCreateInfo.format = get_format(textureInfo->format);
	outCreateInfo.arrayLayers = textureInfo->layersCount;
	outCreateInfo.mipLevels = textureInfo->mipLevels;
	outCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	const uint32_t depth = std::max(1u, textureInfo->depth);
	outCreateInfo.extent = VkExtent3D{ textureInfo->width, textureInfo->height, depth };
	outCreateInfo.samples = get_sample_count(textureInfo->samplesCount);
	
	if (has_flag(textureInfo->resourceFlags, rhi::ResourceFlags::CUBE_TEXTURE))
	{
		outCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	if (has_flag(textureInfo->textureUsage, rhi::ResourceUsage::STORAGE_TEXTURE) &&
		rhi::Utils::is_format_srgb(textureInfo->format))
	{
		outCreateInfo.flags |= VK_IMAGE_CREATE_EXTENDED_USAGE_BIT;
	}

	VkImageUsageFlags imgUsage = get_image_usage(textureInfo->textureUsage);
	outCreateInfo.usage = imgUsage;
	outCreateInfo.imageType = get_image_type(textureInfo->textureDimension);

	VmaAllocationCreateInfo allocCreateInfo{};
	allocCreateInfo.usage = get_memory_usage(textureInfo->memoryUsage);
	
		switch (textureInfo->memoryUsage)
    	{
    		case rhi::MemoryUsage::CPU:
    		case rhi::MemoryUsage::CPU_TO_GPU:
    			allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    			break;
    		case rhi::MemoryUsage::GPU_TO_CPU:
    			allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    			break;
    		default:
    			allocCreateInfo.flags = 0;
    			break;
    	}

	if (_image == VK_NULL_HANDLE)
		destroy(device);
	
	_extent = outCreateInfo.extent;
	_mipLevels = outCreateInfo.mipLevels;
	
	VK_CHECK(vmaCreateImage(device->get_allocator(), &outCreateInfo, &allocCreateInfo, &_image, &_allocation, nullptr));
}

void VulkanTexture::create_texture(VulkanDevice* device, const VkImageCreateInfo& imageCreateInfo)
{
	if (_image == VK_NULL_HANDLE)
		destroy(device);
	
	VmaAllocationCreateInfo allocCreateInfo{};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	
	_extent = imageCreateInfo.extent;
	_mipLevels = imageCreateInfo.mipLevels;
	
	VK_CHECK(vmaCreateImage(device->get_allocator(), &imageCreateInfo, &allocCreateInfo, &_image, &_allocation, nullptr));

}
