#pragma once

#include "engine_core/texture/texture.h"

namespace ad_astris::resource::impl
{
	class TextureImporter
	{
		public:
			static void init();
			static void import(const io::URI& path, ecore::TextureInfo& outTextureInfo);
	};
}