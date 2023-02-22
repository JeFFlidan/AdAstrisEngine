#include "vulkan_texture.h"
#include "vulkan_common.h"
#include "profiler/logger.h"

using namespace ad_astris;

vulkan::VulkanTexture::VulkanTexture(VkImageCreateInfo info, VmaAllocator* allocator, VmaMemoryUsage memoryUsage)
{
	allocate_texture(info, allocator, memoryUsage);
}

void vulkan::VulkanTexture::create_texture(VkImageCreateInfo info, VmaAllocator* allocator, VmaMemoryUsage memoryUsage)
{
	allocate_texture(info, allocator, memoryUsage);
}

void vulkan::VulkanTexture::allocate_texture(VkImageCreateInfo info, VmaAllocator* allocator, VmaMemoryUsage memoryUsage)
{
	this->_extent = info.extent;
	this->_mipLevels = info.mipLevels;

	VmaAllocationCreateInfo imgAllocInfo{};
	imgAllocInfo.usage = memoryUsage;
	imgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);	// VMA allocate the image into VRAM in all situations

	vmaCreateImage(*allocator, &info, &imgAllocInfo, &_image, &_allocation, nullptr);
}
