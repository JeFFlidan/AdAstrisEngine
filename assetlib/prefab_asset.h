#pragma once

#include <asset_loader.h>
#include <array>

namespace assets
{
	struct PrefabInfo
	{
		std::string materialPath;
		std::string meshPath;
		std::array<float, 16> matrix;
	};

	PrefabInfo read_prefab_info(AssetFile* file);
	AssetFile pack_prefab(PrefabInfo* info);
}