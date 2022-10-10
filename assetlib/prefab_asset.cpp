#include <json.hpp>
#include <iostream>

#include <asset_loader.h>
#include <prefab_asset.h>

namespace assets
{
	PrefabInfo read_prefab_info(AssetFile* file)
	{
		PrefabInfo info;
		nlohmann::json prefabMetaData = nlohmann::json::parse(file->json);

		info.meshPath = prefabMetaData["mesh_path"];
		info.materialPath = prefabMetaData["material_path"];

		for (int i = 0; i != 16; ++i)
		{
			info.matrix[i] = file->binaryBlob[i];
		}

		return info;
	}

	AssetFile pack_prefab(PrefabInfo* info)
	{
		nlohmann::json prefabMetaData;
		prefabMetaData["mesh_path"] = info->meshPath;
		prefabMetaData["material_path"] = info->materialPath;

		AssetFile file;
		file.type[0] = 'P';
		file.type[1] = 'R';
		file.type[2] = 'E';
		file.type[3] = 'F';
		file.version = 0;
		
		file.json = prefabMetaData.dump();

		file.binaryBlob.resize(16);
		memcpy(file.binaryBlob.data(), (char*)info->matrix.data(), sizeof(float) * 16);

		return file;
	}
}
