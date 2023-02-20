#include "vulkan_texture.h"
#include "vulkan_common.h"

using namespace ad_astris;

vulkan::VulkanTexture::VulkanTexture(
	VmaAllocator* allocator,
	VmaMemoryUsage memoryUsage,
	VkExtent3D extent,
	uint8_t mipLevels,
	uint8_t layersCount,
	VkFormat format,
	VkImageUsageFlags imageUsage,
	VkSampleCountFlagBits samplesCount,
	VkImageType imageType)
{
	allocate_texture(allocator, memoryUsage, extent, mipLevels, layersCount, format, imageUsage, samplesCount, imageType);
}

void vulkan::VulkanTexture::create_texture(
	VmaAllocator* allocator,
	VmaMemoryUsage memoryUsage,
	VkExtent3D extent,
	uint8_t mipLevels,
	uint8_t layersCount,
	VkFormat format,
	VkImageUsageFlags imageUsage,
	VkSampleCountFlagBits samplesCount,
	VkImageType imageType)
{
	allocate_texture(allocator, memoryUsage, extent, mipLevels, layersCount, format, imageUsage, samplesCount, imageType);
}

void vulkan::VulkanTexture::allocate_texture(
	VmaAllocator* allocator,
	VmaMemoryUsage memoryUsage,
	VkExtent3D ext,
	uint8_t mipLvl,
	uint8_t layersCount,
	VkFormat format,
	VkImageUsageFlags imageUsage,
	VkSampleCountFlagBits samplesCount,
	VkImageType imageType)
{
	this->_extent = ext;
	this->_mipLevels = mipLvl;
	
	VkImageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.format = format;
	createInfo.arrayLayers = layersCount;
	createInfo.mipLevels = _mipLevels;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.extent = _extent;
	createInfo.samples = samplesCount;
	createInfo.usage = imageUsage | VK_IMAGE_USAGE_SAMPLED_BIT;
	createInfo.imageType = imageType;

	VmaAllocationCreateInfo imgAllocInfo{};
	imgAllocInfo.usage = memoryUsage;
	imgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);	// VMA allocate the image into VRAM in all situations

	vmaCreateImage(*allocator, &createInfo, &imgAllocInfo, &_image, &_allocation, nullptr);
}
