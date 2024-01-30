#include "level.h"
#include "engine_core/world.h"
#include "file_system/utils.h"
#include "resource_manager/resource_visitor.h"
#include "engine_core/basic_events.h"
#include "core/constants.h"

using namespace ad_astris;
using namespace ecore;

Level::Level()
{
	_isDirty = true;
}

Level::Level(io::URI& path, ObjectName* levelName)
{
	_levelInfo.uuid = UUID();
	_name = levelName;
	_path = path;
	_isDirty = true;
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
	
	levelMainJson["level_metadata"] = level::Utils::pack_level_info(&_levelInfo);

	nlohmann::json jsonForEntities;
	level::Utils::build_json_from_entities(jsonForEntities, this);
	levelMainJson["entities"] = jsonForEntities;
	std::string newMetadata = levelMainJson.dump(JSON_INDENT);
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
	nlohmann::json levelMetadata = levelMainJson["level_metadata"];
	
	_levelInfo = level::Utils::unpack_level_info(levelMetadata);
	
	_entitiesJson = levelMainJson["entities"];
}

void Level::build_entities()
{
	if (!_entitiesJson.empty())
	{
		level::Utils::build_entities_from_json(_entitiesJson, this);
		_entitiesJson.clear();
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
