#include "vulkan_texture.h"
#include "vulkan_device.h"
#include "vulkan_common.h"
#include "profiler/logger.h"

using namespace ad_astris;

vulkan::VulkanTexture::VulkanTexture(VkImageCreateInfo info, VmaAllocator* allocator, VmaMemoryUsage memoryUsage)
{
	allocate_texture(info, allocator, memoryUsage);
}

void vulkan::VulkanTexture::destroy_texture(VmaAllocator allocator)
{
	vmaDestroyImage(allocator, _image, _allocation);
	_image = VK_NULL_HANDLE;
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

vulkan::VulkanTextureView::VulkanTextureView(VulkanDevice* device, VkImageViewCreateInfo& info)
{
	VK_CHECK(vkCreateImageView(device->get_device(), &info, nullptr, &_imageView));
}

void vulkan::VulkanTextureView::create(VulkanDevice* device, VkImageViewCreateInfo& info)
{
	VK_CHECK(vkCreateImageView(device->get_device(), &info, nullptr, &_imageView));
}

void vulkan::VulkanTextureView::destroy(VulkanDevice* device)
{
	if (_imageView != VK_NULL_HANDLE)
		vkDestroyImageView(device->get_device(), _imageView, nullptr);
	_imageView = VK_NULL_HANDLE;
}

vulkan::VulkanSampler::VulkanSampler(VulkanDevice* device, VkSamplerCreateInfo& info)
{
	VK_CHECK(vkCreateSampler(device->get_device(), &info, nullptr, &_sampler));
}

void vulkan::VulkanSampler::destroy(VulkanDevice* device)
{
	if (_sampler != VK_NULL_HANDLE)
		vkDestroySampler(device->get_device(), _sampler, nullptr);
	_sampler = VK_NULL_HANDLE;
}
