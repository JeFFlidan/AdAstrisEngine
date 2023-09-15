#include "entity_manager.h"
#include "serializers.h"
#include "core/utils.h"

using namespace ad_astris::ecs;

ArchetypeHandle EntityManager::create_archetype(ArchetypeCreationContext& context)
{
	if (context._componentIDs.empty())
	{
		LOG_ERROR("EntityManager::create_archetype(): No component IDs to create archetype")
		return ArchetypeHandle(-1);
	}

	size_t idsHash = CoreUtils::hash_numeric_vector(context._componentIDs);
	size_t tagsHash = CoreUtils::hash_numeric_vector(context._tagIDs);
	size_t mainHash = idsHash ^ tagsHash;
	
	//auto inArchIt = _componentsHashToArchetypeId.find(mainHash);  Have a crush hear without any reason. In the future maybe I will investigate it.

	std::scoped_lock<std::mutex> locker(_archetypeMutex);
	if (_componentsHashToArchetypeId.find(mainHash) != _componentsHashToArchetypeId.end())
	{
		return ArchetypeHandle(_componentsHashToArchetypeId.find(mainHash)->second);
	}
	
	Archetype archetype(context);
	uint32_t archetypeId = _archetypes.size();
	_archetypes.push_back(archetype);
	_lastCreatedArchetypes.push_back(archetypeId);
	_componentsHashToArchetypeId[mainHash] = archetypeId;

	return ArchetypeHandle(archetypeId);
}

ArchetypeHandle EntityManager::create_archetype(ArchetypeExtensionContext& context)
{
	// TODO Maybe it's better to use lock after merging vectors
	std::scoped_lock<std::mutex> locker(_archetypeMutex);
	Archetype& oldArchetype = _archetypes[context._srcArchetype.get_id()];
	ChunkStructure& oldArchetypeChunkStructure = oldArchetype._chunkStructure;
	
	std::vector<uint32_t> newComponentTypeIDs;
	size_t componentsHash = merge_type_ids_vectors(
		newComponentTypeIDs,
		oldArchetypeChunkStructure.componentIds,
		context._componentIDs);

	std::vector<uint32_t> newTagTypeIDs;
	size_t tagsHash = merge_type_ids_vectors(
		newTagTypeIDs,
		oldArchetypeChunkStructure.tagIDs,
		context._tagIDs);
	
	size_t newHash = componentsHash ^ tagsHash;

	auto inArchIt = _componentsHashToArchetypeId.find(newHash);
	if (inArchIt != _componentsHashToArchetypeId.end())
	{
		return ArchetypeHandle(inArchIt->first);
	}
	
	auto newComponentIdToSize = oldArchetype._chunkStructure.componentIdToSize;
	newComponentIdToSize.insert(context._idToSize.begin(), context._idToSize.end());
	
	uint32_t newAllComponentsSize = 0;
	for (auto& idToSize : newComponentIdToSize)
	{
		newAllComponentsSize += idToSize.second;
	}
	
	ArchetypeCreationContext creationContext;
	creationContext._componentIDs = std::move(newComponentTypeIDs);
	creationContext._idToSize = std::move(newComponentIdToSize);
	creationContext._allComponentsSize = newAllComponentsSize;
	creationContext._tagIDs = std::move(newTagTypeIDs);

	Archetype newArchetype{ creationContext };
	uint32_t newArchetypeId = _archetypes.size();
	_archetypes.push_back(newArchetype);
	_lastCreatedArchetypes.push_back(newArchetypeId);
	_componentsHashToArchetypeId[newHash] = newArchetypeId;

	return ArchetypeHandle(newArchetypeId);
}

ArchetypeHandle EntityManager::create_archetype(ArchetypeReductionContext context)
{
	// TODO
	return ArchetypeHandle();
}

ArchetypeHandle EntityManager::get_entity_archetype(Entity& entity)
{
	return { _entityToItsInfoInArchetype[entity].archetypeId };
}

Entity EntityManager::create_entity()
{
	//TODO
	return Entity();
}

Entity EntityManager::create_entity(ArchetypeHandle& archetypeHandle)
{
	std::scoped_lock<std::mutex> locker(_entityMutex);
	Entity entity{ UUID() };
	Archetype& archetype = _archetypes[archetypeHandle.get_id()];
	uint32_t columnIndex = archetype.add_entity(entity);
	EntityInArchetypeInfo entityInArchetype;
	entityInArchetype.column = columnIndex;
	entityInArchetype.archetypeId = archetypeHandle.get_id();
	_entityToItsInfoInArchetype[entity] = entityInArchetype;
	return entity;
}

Entity EntityManager::create_entity(EntityCreationContext& entityContext, UUID uuid)
{
	std::vector<uint32_t> componentIdsToMove;
	copy_vector(entityContext._componentIDs, componentIdsToMove);

	std::vector<uint32_t> tagIDsToMove;
	copy_vector(entityContext._tagIDs, tagIDsToMove);
	
	ArchetypeCreationContext archetypeContext;
	archetypeContext._componentIDs = std::move(componentIdsToMove);
	archetypeContext._idToSize = entityContext._typeIdToSize;
	archetypeContext._allComponentsSize = entityContext._allComponentsSize;
	archetypeContext._tagIDs = std::move(tagIDsToMove);
	
	ArchetypeHandle archHandle = create_archetype(archetypeContext);

	UUID newUUID = uuid ? uuid : UUID();
	Entity entity{ newUUID };

	std::scoped_lock<std::mutex> locker(_entityMutex);
	Archetype& archetype = _archetypes[archHandle.get_id()];
	uint32_t entityColumn = archetype.add_entity(entity);
	EntityInArchetypeInfo entityInArchetype;
	entityInArchetype.column = entityColumn;
	entityInArchetype.archetypeId = archHandle.get_id();
	_entityToItsInfoInArchetype[entity] = entityInArchetype;

	archetype.set_components(entity, entityColumn, entityContext);

	return entity;
}

Entity EntityManager::build_entity_from_json(UUID& uuid, std::string& json)
{
	nlohmann::json entityJson = nlohmann::json::parse(json);
	std::string componentsStr = entityJson["components"];
	nlohmann::json jsonWithComponents = nlohmann::json::parse(componentsStr);
	std::vector<std::string> tagNames = entityJson["tags"].get<std::vector<std::string>>();
	EntityCreationContext creationContext;
	
	for (auto& componentInfo : jsonWithComponents.items())
	{
		std::string componentName = componentInfo.key();
		uint32_t typeId = TypeInfoTable::get_component_id(componentName);
		serializers::BaseSerializer* serializer = serializers::get_table()->get_serializer(typeId);
		serializer->deserialize(creationContext, componentInfo.value());
	}

	for (auto& tagName : tagNames)
	{
		creationContext._tagIDs.push_back(TypeInfoTable::get_tag_id(tagName));
	}

	Entity entity = create_entity(creationContext, uuid);
	return entity;
}

void EntityManager::build_components_json_from_entity(Entity& entity, nlohmann::json& levelJson)
{
	EntityInArchetypeInfo entityInArchetype = _entityToItsInfoInArchetype[entity];
	Archetype& archetype = _archetypes[entityInArchetype.archetypeId];
	uint32_t column = entityInArchetype.column;
	uint32_t componentsNumberInArchetype = archetype._chunkStructure.componentIds.size();
	uint8_t componentPtr[constants::MAX_COMPONENT_SIZE];
	
	nlohmann::json componentsJson;
	
	for (int i = 0; i != componentsNumberInArchetype; ++i)
	{
		//componentsArray += i * constants::MAX_COMPONENT_SIZE;
		uint32_t typeId = archetype._chunkStructure.componentIds[i];
		archetype.get_component_by_component_type_id(entity, column, typeId, componentPtr);
		if (serializers::get_table()->has_serializer(typeId))
		{
			serializers::BaseSerializer* serializer = serializers::get_table()->get_serializer(typeId);
			serializer->serialize(componentPtr, componentsJson);
		}
	}

	nlohmann::json entityJson;
	entityJson["components"] = componentsJson.dump();

	std::vector<uint32_t>& tagIDs = archetype._chunkStructure.tagIDs;
	std::vector<std::string> tagNames;
	tagNames.reserve(tagIDs.size());
	for (auto& tagID : tagIDs)
	{
		tagNames.push_back(TypeInfoTable::get_tag_name(tagID));
	}

	entityJson["tags"] = tagNames;
	std::scoped_lock<std::mutex> locker(_entityMutex);
	levelJson[std::to_string(entity.get_uuid())] = entityJson.dump();
}

void EntityManager::destroy_entity(Entity& entity)
{
	auto entityIterator = _entityToItsInfoInArchetype.find(entity);
	if (entityIterator == _entityToItsInfoInArchetype.end())
		return;

	EntityInArchetypeInfo entityInArchetype = entityIterator->second;
	Archetype& archetype = _archetypes[entityInArchetype.archetypeId];
	archetype.destroy_entity(entity, entityInArchetype.column);
	_entityToItsInfoInArchetype.erase(entityIterator);
}

size_t EntityManager::merge_type_ids_vectors(
	std::vector<uint32_t>& dstTypeIDs,
	std::vector<uint32_t>& srcTypeIDs,
	std::vector<uint32_t>& contextTypeIDs)
{
	dstTypeIDs.resize(srcTypeIDs.size() + contextTypeIDs.size());
	std::merge(contextTypeIDs.begin(), contextTypeIDs.end(), srcTypeIDs.begin(), srcTypeIDs.end(), dstTypeIDs.begin());
	auto last = std::unique(dstTypeIDs.begin(), dstTypeIDs.end());
	dstTypeIDs.erase(last);
	return CoreUtils::hash_numeric_vector(dstTypeIDs);
}

void EntityManager::copy_vector(
	std::vector<uint32_t>& srcTypeIDs,
	std::vector<uint32_t>& dstTypeIDs)
{
	if (!std::is_sorted(srcTypeIDs.begin(), srcTypeIDs.end()))
		std::sort(srcTypeIDs.begin(), srcTypeIDs.end());
	dstTypeIDs.resize(srcTypeIDs.size());
	memcpy(dstTypeIDs.data(), srcTypeIDs.data(), srcTypeIDs.size() * sizeof(uint32_t));
}