#pragma once

#include "engine_core/model/model.h"
#include "engine_core/texture/texture.h"
#include "engine_core/material/material.h"

namespace ad_astris::resource::impl
{
	class OBJImporter
	{
		public:
			static bool import(
				const std::string& path,
				std::vector<ecore::ModelInfo>& outModelInfos,
				std::vector<ecore::TextureInfo>& outTextureInfos,
				std::vector<ecore::MaterialInfo>& materialInfos,
				const ecore::ModelConversionContext& conversionContext);
	};
}