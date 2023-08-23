#pragma once

#include "engine_core/uuid.h"
#include <vector>
#include <string>

namespace ad_astris::resource
{
	enum class ResourceType
	{
		UNDEFINED,
		MODEL,
		TEXTURE,
		LEVEL,
		MATERIAL,
		MATERIAL_TEMPLATE,
		SHADER
	};
	
	struct LevelEngineInfo
	{
		UUID uuid;
		std::vector<UUID> texturesUUID;
	};

	struct LevelFileInfo
	{
		UUID uuid;
		std::vector<std::string> modelInstancesInfo;
		std::vector<std::string> texturesInfo;
	};
}