#pragma once

#include "common.h"

namespace ad_astris::resource::impl
{
	class FBXImporter
	{
		public:
			static bool import(
				const std::string& path,
				std::vector<ModelCreateInfo>& outModelInfos,
				std::vector<TextureCreateInfo>& outTextureInfos,
				std::vector<MaterialCreateInfo>& materialInfos,
				const ecore::ModelConversionContext& conversionContext);
	};
}