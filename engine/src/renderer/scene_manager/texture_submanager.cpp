#include "texture_submanager.h"
#include <cmath>

using namespace ad_astris::renderer::impl;


uint32_t TextureSubmanager::get_mip_levels(uint32_t width, uint32_t height)
{
	return std::floor(std::log2(std::min(width, height))) + 1;
}
