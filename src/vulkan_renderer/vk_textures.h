#pragma once

#include "vk_types.h"
#include "vk_renderer.h"
#include <texture_asset.h>

namespace ad_astris
{
	AllocatedImage upload_image(VkRenderer& engine, AllocatedBuffer& stagingBuffer, assets::TextureInfo& textureInfo, VkFormat image_format);
	bool load_image_from_file(VkRenderer& engine, const char* file, AllocatedImage& outImage);
	bool load_image_from_asset(VkRenderer& engine, const char* filename, AllocatedImage& outImage);
}