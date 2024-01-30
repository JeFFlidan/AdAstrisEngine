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

void Utils::build_entities_from_json(nlohmann::json& entitiesInfo, Level* level)
{
	ecs::EntityManager* entityManager = level->_entityManager;
	std::vector<ecs::Entity>& entities = level->_entities;

	// TODO Think about async
	for (auto& info : entitiesInfo.items())
	{
		UUID uuid(std::stoull(info.key()));
		nlohmann::json componentsJson = info.value();
		ecs::Entity entity = entityManager->build_entity_from_json(uuid, componentsJson);
		entities.push_back(entity);
		EntityCreatedEvent event(entity, entityManager);
		level->get_owning_world()->get_event_manager()->enqueue_event(event);
	}
}

void Utils::build_json_from_entities(nlohmann::json& jsonForEntities, Level* level)
{
	ecs::EntityManager* entityManager = level->_entityManager;
	std::vector<ecs::Entity>& entities = level->_entities;

	for (auto& entity : entities)
	{
		entityManager->build_components_json_from_entity(entity, jsonForEntities);
	}
}
