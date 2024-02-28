#pragma once

#include "engine_core/texture/texture2D.h"

namespace ad_astris::renderer::impl
{
	class TextureSubmanager
	{
		public:
			TextureSubmanager();

		private:
			void generate_new_mipmaps(ecore::Texture2DHandle texture2DHandle);
			void setup_generated_mipmaps(ecore::Texture2DHandle texture2DHandle);
			uint32_t get_mip_levels(uint32_t width, uint32_t height);
	};
}