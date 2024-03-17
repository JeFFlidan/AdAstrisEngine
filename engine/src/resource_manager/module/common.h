#pragma once

#include "engine_core/model/model.h"
#include "engine_core/texture/texture.h"
#include "engine_core/material/material.h"

namespace ad_astris::resource::impl
{
	struct ModelCreateInfo
	{
		ecore::ModelInfo info;
		std::string name;
	};

	struct TextureCreateInfo
	{
		ecore::TextureInfo info;
		std::string name;
	};

	struct MaterialCreateInfo
	{
		ecore::MaterialInfo info;
		std::string name;
	};
}