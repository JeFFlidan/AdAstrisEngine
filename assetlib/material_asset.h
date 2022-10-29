#pragma once

#include <asset_loader.h>
#include <unordered_map>

namespace assets
{
	using un_str_map = std::unordered_map<std::string, std::string>;

	enum class MaterialMode : uint8_t
	{
		NONE = 0,
		OPAQUE = 1,
		TRANSPARENT = 2,
		MASKED = 3
	};

	struct MaterialInfo
	{
		std::string baseEffect;
		std::string materialName;
		un_str_map textures;
		un_str_map customProperties;
		MaterialMode mode;
	};

	MaterialInfo read_material_info(AssetFile* file);
	AssetFile pack_material(MaterialInfo* info);
}
