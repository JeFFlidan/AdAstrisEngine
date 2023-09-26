#include "level.h"
#include "engine_core/world.h"
#include "file_system/utils.h"
#include "resource_manager/resource_visitor.h"
#include "engine_core/basic_events.h"

using namespace ad_astris;
using namespace ecore;

Level::Level(io::URI& path, ObjectName* levelName)
{
	_levelInfo.uuid = UUID();
	_name = levelName;
	_path = path;
}

World* Level::get_owning_world()
{
	return _owningWorld;
}

void Level::set_owning_world(World* world)
{
	_owningWorld = world;
	_entityManager = _owningWorld->get_entity_manager();
}

ecs::EntityManager* Level::get_entity_manager()
{
	return _entityManager;
}

void Level::add_entity(ecs::Entity& entity)
{
	_entities.push_back(entity);
	EntityCreatedEvent event(entity, _entityManager);
	_owningWorld->get_event_manager()->enqueue_event(event);
}

ecs::Entity Level::create_entity(ecs::EntityCreationContext& creationContext)
{
	ecs::Entity entity = _entityManager->create_entity(creationContext);
	EntityCreatedEvent event(entity, _entityManager);
	_owningWorld->get_event_manager()->enqueue_event(event);
	return entity;
}

void Level::serialize(io::File* file)
{
	nlohmann::json levelMainJson;

	std::string levelMetadata = level::Utils::pack_level_info(&_levelInfo);
	levelMainJson["level_metadata"] = levelMetadata;

	nlohmann::json jsonForEntities;
	LOG_INFO("AFTER BUILDING JSON {}", _entities.size())
	level::Utils::build_json_from_entities(jsonForEntities, this);
	LOG_INFO("AFTER BUILDING JSON")
	levelMainJson["entities"] = jsonForEntities.dump();
	std::string newMetadata = levelMainJson.dump();
	file->set_metadata(newMetadata);
}

void Level::deserialize(io::File* file, ObjectName* objectName)
{
	if (!objectName)
	{
		LOG_ERROR("Level::deserialize(): Can't load level withoud name")
		return;
	}

	_name = objectName;
	_path = file->get_file_path();
	
	nlohmann::json levelMainJson = nlohmann::json::parse(file->get_metadata());
	std::string levelMetadata = levelMainJson["level_metadata"];
	
	_levelInfo = level::Utils::unpack_level_info(levelMetadata);
	
	_entitiesJsonStr = levelMainJson["entities"];
}

void Level::build_entities()
{
	if (!_entitiesJsonStr.empty())
	{
		level::Utils::build_entities_from_json(_entitiesJsonStr, this);
		_entitiesJsonStr.clear();
	}
}

uint64_t Level::get_size()
{
	return 0;
}

bool Level::is_resource()
{
	return false;
}

UUID Level::get_uuid()
{
	return _levelInfo.uuid;
}

std::string Level::get_description()
{
	return std::string();
}

std::string Level::get_type()
{
	return "level";
}

void Level::accept(resource::IResourceVisitor& resourceVisitor)
{
	resourceVisitor.visit(this);
}
