#pragma once

#include "common.h"

namespace ad_astris::resource::impl
{
	struct ModelImportContext
	{
		ModelImportContext(void* conversionContext);
		
		std::vector<ModelCreateInfo> modelCreateInfos;
		std::vector<TextureCreateInfo> textureCreateInfos;
		std::vector<MaterialCreateInfo> materialCreateInfos;
		ecore::ModelConversionContext* conversionContext{ nullptr };
	};
	
	class ModelImporter
	{
		public:
			static bool import(const io::URI& path, ModelImportContext& context);
	};
}