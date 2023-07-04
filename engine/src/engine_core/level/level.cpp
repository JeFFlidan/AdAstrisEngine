#include "level.h"
#include "engine_core/world.h"
#include "file_system/utils.h"

using namespace ad_astris;
using namespace ad_astris::ecore;

Level::Level(io::URI& path, ObjectName* levelName)
{
	_levelInfo.uuid = UUID();
	_name = levelName;
	_path = path;
}

Level::Level(void* entityManager)
{
	_entityManager = static_cast<ecs::EntityManager*>(entityManager);
}

ecs::EntityManager* Level::get_entity_manager()
{
	return _entityManager;
}

void Level::add_entity(ecs::Entity& entity)
{
	_entities.push_back(entity);
}

void Level::serialize(io::IFile* file)
{
	nlohmann::json levelMainJson;

	std::string levelMetadata = level::Utils::pack_level_info(&_levelInfo);
	levelMainJson["level_metadata"] = levelMetadata;

	nlohmann::json jsonForEntities;
	level::Utils::build_json_from_entities(jsonForEntities, this);
	levelMainJson["entities"] = jsonForEntities.dump();

	std::string newMetadata = levelMainJson.dump();
	file->set_metadata(newMetadata);
}

void Level::deserialize(io::IFile* file, ObjectName* objectName)
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
	
	std::string entitiesInfo = levelMainJson["entities"];
	level::Utils::build_entities_from_json(entitiesInfo, this);
}

uint64_t Level::get_size()
{
	// TODO
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
	// TODO
}

std::string Level::get_type()
{
	return "level";
}

void Level::rename_in_engine(ObjectName& newName)
{
	
}
