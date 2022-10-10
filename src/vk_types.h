// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

struct AllocatedBuffer
{
	VkBuffer _buffer;
	VmaAllocation _allocation;
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

enum class MeshpassType : uint8_t
{
	None = 0,
	Forward = 1,
	Transparency = 2,
	DirectionalShadow = 3
};

//we will add our main reusable types here
