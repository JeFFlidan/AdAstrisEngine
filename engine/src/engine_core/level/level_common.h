#pragma once

#include <ecs.h>

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
			static std::string pack_level_info(LevelInfo* levelInfo);
			static LevelInfo unpack_level_info(std::string& strMetadata);
			static void build_entities_from_json(std::string& entitiesInfo, Level* level);
			static void build_json_from_entities(nlohmann::json& jsonForEntities, Level* level);
	};
}