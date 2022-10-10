#pragma once

#include <vk_types.h>
#include <vk_engine.h>
#include <texture_asset.h>

namespace vkutil
{
	AllocatedImage upload_image(VulkanEngine& engine, AllocatedBuffer& stagingBuffer, assets::TextureInfo& textureInfo, VkFormat image_format);
	bool load_image_from_file(VulkanEngine& engine, const char* file, AllocatedImage& outImage);
	bool load_image_from_asset(VulkanEngine& engine, const char* filename, AllocatedImage& outImage);
}