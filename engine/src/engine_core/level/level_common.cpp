#include "level.h"
#include "level_common.h"
#include "engine_core/basic_events.h"
#include "core/constants.h"

#include <json/json.hpp>

#include "engine_core/world.h"

using namespace ad_astris::ecore::level;

nlohmann::json Utils::pack_level_info(LevelInfo* levelInfo)
{
	nlohmann::json levelMetadata;
	levelMetadata["uuid"] = (uint64_t)levelInfo->uuid;
	return levelMetadata;
}

LevelInfo Utils::unpack_level_info(const nlohmann::json& metadata)
{
	LevelInfo info;
	info.uuid = UUID((uint64_t)metadata["uuid"]);
	return info;
}
