#pragma once

#include <cstdint>
#include <string>

namespace ad_astris::devtools::pl_impl
{
	struct TextureInfo
	{
		int32_t width;
		int32_t height;
		uint32_t textureID;
	};
	
	class Utils
	{
		public:
			static bool load_texture_from_file(const std::string& filePath, TextureInfo& textureInfo);
	};
}