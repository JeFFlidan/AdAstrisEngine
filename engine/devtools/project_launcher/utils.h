#pragma once

#include "ui_core/common.h"
#include <string>

namespace ad_astris::devtools::pl_impl
{
	class Utils
	{
		public:
			static bool load_texture_from_file(const std::string& filePath, uicore::TextureInfo& textureInfo);
	};
}