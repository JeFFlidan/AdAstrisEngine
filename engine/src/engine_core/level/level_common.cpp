#include "level.h"
#include "level_common.h"
#include "engine_core/basic_events.h"

#include <json/json.hpp>

#include "engine_core/world.h"

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
	info.uuid = UUID((uint64_t)levelMetadata["uuid"]);
	return info;
}

void Utils::build_entities_from_json(std::string& entitiesInfo, Level* level)
{
	ecs::EntityManager* entityManager = level->_entityManager;
	std::vector<ecs::Entity>& entities = level->_entities;

	nlohmann::json entitiesInfoJson = nlohmann::json::parse(entitiesInfo);

	// TODO Think about async
	for (auto& info : entitiesInfoJson.items())
	{
		UUID uuid(std::stoull(info.key()));
		std::string componentsJson = info.value();
		ecs::Entity entity = entityManager->build_entity_from_json(uuid, componentsJson);
		entities.push_back(entity);
		EntityCreatedEvent event(entity, entityManager);
		LOG_INFO("")
		level->get_owning_world()->get_event_manager()->enqueue_event(event);
	}
}

void Utils::build_json_from_entities(nlohmann::json& jsonForEntities, Level* level)
{
	ecs::EntityManager* entityManager = level->_entityManager;
	std::vector<ecs::Entity>& entities = level->_entities;

	for (auto& entity : entities)
	{
		LOG_INFO("ENTITY {}", entity.get_uuid())
		entityManager->build_components_json_from_entity(entity, jsonForEntities);
		LOG_INFO("ENTITY FINISH")
	}
}
