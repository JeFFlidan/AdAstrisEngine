#include "level.h"
#include "engine_core/world.h"
#include "file_system/utils.h"
#include "resource_manager/resource_visitor.h"
#include "engine_core/basic_events.h"
#include "core/global_objects.h"
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

Level::Level(const LevelInfo& info, ObjectName* name) : _info(info)
{
	_name = name;
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
	std::vector<uint8_t> componentsBlob;

	for (auto& entity : _entities)
	{
		_entityManager->serialize_entity(entity, jsonForEntities, componentsBlob);
	}
	
	levelMainJson["entities"] = jsonForEntities;
	std::string newMetadata = levelMainJson.dump(JSON_INDENT);
	file->set_metadata(newMetadata);

	// TEMP!!!
	uint8_t* blob = new uint8_t[componentsBlob.size()];
	memcpy(blob, componentsBlob.data(), componentsBlob.size());
	file->set_binary_blob(blob, componentsBlob.size());
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
	_componentsData.resize(file->get_binary_blob_size());
	memcpy(_componentsData.data(), file->get_binary_blob(), file->get_binary_blob_size());
}

void Level::build_entities()
{
	if (!_entitiesJson.empty())
	{
		for (auto& info : _entitiesJson.items())
		{
			UUID uuid(std::stoull(info.key()));
			nlohmann::json componentsJson = info.value();
			ecs::Entity entity = _entityManager->deserialize_entity(uuid, componentsJson, _componentsData);
			_entities.push_back(entity);
			EntityCreatedEvent event(entity, _entityManager);
			EVENT_MANAGER()->enqueue_event(event);
		}
		
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
