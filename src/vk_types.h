// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <stdint.h>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

class VulkanEngine;

struct AllocatedBuffer
{
	VkBuffer _buffer;
	VmaAllocation _allocation;

	size_t _bufferSize = 0;

	static AllocatedBuffer create_buffer(VulkanEngine* engine, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	void copy_from(VulkanEngine* engine, void* srcBuffer, size_t sizeInBytes);
	void destroy_buffer(VulkanEngine* engine);
	static void copyBufferCmd(VulkanEngine* engine, 
		VkCommandBuffer cmd, 
		AllocatedBuffer* srcBuffer, 
		AllocatedBuffer* dstBuffer, 
		VkDeviceSize dstOffset = 0, 
		VkDeviceSize srcOffset = 0);
};

struct AllocatedImage
{
	VkImage _image;
	VmaAllocation _allocation;
};

struct Texture
{
	AllocatedImage image;
	VkImageView imageView;
};

struct Attachment
{
	VkImageView _imageView;
	AllocatedImage _imageData;
	VkFormat _format;
};

