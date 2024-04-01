#pragma once

#include "ecs/ecs.h"

namespace ad_astris::ecore
{
	class Level;
}

namespace ad_astris::ecore::level
{
	struct LevelInfo
	{
		UUID uuid;
	};
	
	class Utils
	{
		public:
			static nlohmann::json pack_level_info(LevelInfo* levelInfo);
			static LevelInfo unpack_level_info(const nlohmann::json& metadata);
	};
}