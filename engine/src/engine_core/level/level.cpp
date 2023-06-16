#include "level.h"
#include "file_system/utils.h"

using namespace ad_astris;

ecore::Level::Level(io::URI& path)
{
	_levelInfo.uuid = UUID();
	std::string levelName = io::Utils::get_file_name(path);
	_name = ObjectName(levelName.c_str());
	_path = path;
}

ecs::EntityManager* ecore::Level::get_entity_manager()
{
	return &_world;
}

void ecore::Level::add_entity(ecs::Entity& entity)
{
	_entities.push_back(entity);
}

void ecore::Level::serialize(io::IFile* file)
{
	nlohmann::json levelMainJson;

	std::string levelMetadata = level::Utils::pack_level_info(&_levelInfo);
	LOG_INFO("Level metadata: {}", levelMetadata)
	levelMainJson["level_metadata"] = levelMetadata;

	nlohmann::json jsonForEntities;
	level::Utils::build_json_from_entities(jsonForEntities, this);
	LOG_INFO("Json for entities: {}", jsonForEntities.dump())
	levelMainJson["entities"] = jsonForEntities.dump();

	LOG_INFO("Before dumping")
	std::string newMetadata = levelMainJson.dump();
	LOG_INFO("Length of new metadata: {}", newMetadata)
	LOG_INFO("New metadata: {}", newMetadata)
	file->set_metadata(newMetadata);
}

void ecore::Level::deserialize(io::IFile* file, ObjectName* newName)
{
	if (!newName)
	{
		LOG_ERROR("Level::deserialize(): Can't load level withoud name")
		return;
	}

	_name = *newName;
	_path = file->get_file_path();
	
	nlohmann::json levelMainJson = nlohmann::json::parse(file->get_metadata());
	std::string levelMetadata = levelMainJson["level_metadata"];
	
	_levelInfo = level::Utils::unpack_level_info(levelMetadata);
	
	std::string entitiesInfo = levelMainJson["entities"];
	level::Utils::build_entities_from_json(entitiesInfo, this);
}

uint64_t ecore::Level::get_size()
{
	// TODO
}

bool ecore::Level::is_resource()
{
	return false;
}

UUID ecore::Level::get_uuid()
{
	return _levelInfo.uuid;
}

std::string ecore::Level::get_description()
{
	// TODO
}

std::string ecore::Level::get_type()
{
	return "level";
}

void ecore::Level::rename_in_engine(ObjectName& newName)
{
	
}
