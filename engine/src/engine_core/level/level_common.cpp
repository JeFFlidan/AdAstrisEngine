#include "level.h"
#include "level_common.h"

#include <json.hpp>

using namespace ad_astris::ecore::level;

std::string Utils::pack_level_info(LevelInfo* levelInfo)
{
	nlohmann::json levelMetadata;
	levelMetadata["uuid"] = (uint64_t)levelInfo->uuid;
	return levelMetadata.dump();
}

LevelInfo Utils::unpack_level_info(std::string& strMetadata)
{
	nlohmann::json levelMetadata = nlohmann::json::parse(strMetadata);
	LevelInfo info;
	info.uuid = UUID(levelMetadata["uuid"]);
	return info;
}

void Utils::build_entities_from_json(std::string& entitiesInfo, Level* level)
{
	ecs::EntityManager& world = level->_world;
	std::vector<ecs::Entity>& entities = level->_entities;

	nlohmann::json entitiesInfoJson = nlohmann::json::parse(entitiesInfo);

	// TODO Think about async
	for (auto& info : entitiesInfoJson.items())
	{
		UUID uuid(std::stoull(info.key()));
		std::string componentsJson = info.value();
		ecs::Entity entity = world.build_entity_from_json(uuid, componentsJson);
		entities.push_back(entity);
	}
}

void Utils::build_json_from_entities(nlohmann::json& jsonForEntities, Level* level)
{
	ecs::EntityManager& world = level->_world;
	std::vector<ecs::Entity>& entities = level->_entities;

	for (auto& entity : entities)
	{
		world.build_components_json_from_entity(entity, jsonForEntities);
	}
}
