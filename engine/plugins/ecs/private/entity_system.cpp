#include "entity_system.h"
#include "core/utils.h"

using namespace ad_astris::ecs;

ArchetypeHandle EntitySystem::create_archetype(ArchetypeCreationContext& context)
{
	if (context._ids.empty())
	{
		LOG_ERROR("EntitySystem::create_archetype(): No component IDs to create archetype")
		return ArchetypeHandle(-1);
	}

	size_t idsHash = CoreUtils::hash_numeric_vector(context._ids);
	
	auto inArchIt = _componentsHashToArchetypeId.find(idsHash);
	if (inArchIt != _componentsHashToArchetypeId.end())
	{
		return ArchetypeHandle(inArchIt->second);
	}

	Archetype archetype(context);
	uint32_t archetypeId = _archetypes.size();
	_archetypes.push_back(archetype);
	_componentsHashToArchetypeId[idsHash] = archetypeId;

	return ArchetypeHandle(archetypeId);
}

ArchetypeHandle EntitySystem::create_archetype(ArchetypeExtensionContext& context)
{
	Archetype& oldArchetype = _archetypes[context._srcArchetype.get_id()];
	ChunkStructure& oldArchetypeChunkStructure = oldArchetype._chunkStructure;
	std::vector<uint32_t> newTypeIDs;
	newTypeIDs.resize(oldArchetypeChunkStructure.componentIds.size() + context._ids.size());
	std::vector<uint32_t>& srcTypeIDs = oldArchetypeChunkStructure.componentIds;
	std::vector<uint32_t>& contextTypeIDs = context._ids;
	std::merge(contextTypeIDs.begin(), contextTypeIDs.end(), srcTypeIDs.begin(), srcTypeIDs.end(), newTypeIDs.begin());
	auto last = std::unique(newTypeIDs.begin(), newTypeIDs.end());
	newTypeIDs.erase(last);
	size_t newHash = CoreUtils::hash_numeric_vector(newTypeIDs);

	auto inArchIt = _componentsHashToArchetypeId.find(newHash);
	if (inArchIt != _componentsHashToArchetypeId.end())
	{
		return ArchetypeHandle(inArchIt->first);
	}

	// Have to think about all components size
	uint32_t newAllComponentsSize = context._allComponentsSize + oldArchetype._sizeOfOneColumn;
	
	auto newComponentIdToSize = oldArchetype._chunkStructure.componentIdToSize;
	newComponentIdToSize.insert(context._idToSize.begin(), context._idToSize.end());

	ArchetypeCreationContext creationContext;
	creationContext.add_components_id(newTypeIDs);
	creationContext._idToSize = std::move(newComponentIdToSize);
	creationContext._allComponentsSize = newAllComponentsSize;

	Archetype newArchetype{ creationContext };
	uint32_t newArchetypeId = _archetypes.size();
	_archetypes.push_back(newArchetype);
	_componentsHashToArchetypeId[newHash] = newArchetypeId;

	return ArchetypeHandle(newArchetypeId);
}

ArchetypeHandle EntitySystem::create_archetype(ArchetypeReductionContext context)
{
	// TODO
}

ArchetypeHandle EntitySystem::get_entity_archetype(Entity& entity)
{
	return { _entityToItsInfoInArchetype[entity].archetypeId };
}

Entity EntitySystem::create_entity()
{
	// TODO
}

Entity EntitySystem::create_entity(ArchetypeHandle& archetypeHandle)
{
	Entity entity{ UUID() };
	Archetype& archetype = _archetypes[archetypeHandle.get_id()];
	uint32_t columnIndex = archetype.add_entity(entity);
	EntityInArchetypeInfo entityInArchetype;
	entityInArchetype.column = columnIndex;
	entityInArchetype.archetypeId = archetypeHandle.get_id();
	_entityToItsInfoInArchetype[entity] = entityInArchetype;
	return entity;
}

Entity EntitySystem::create_entity(EntityCreationContext& entityContext, UUID uuid)
{
	std::vector<uint32_t>& componentIdsRef = entityContext._componentsId;
	
	if (!std::is_sorted(componentIdsRef.begin(), componentIdsRef.end()))
		std::sort(componentIdsRef.begin(), componentIdsRef.end());
	std::vector<uint32_t> componentIdsToMove(componentIdsRef.size());
	memcpy(componentIdsToMove.data(), componentIdsRef.data(), componentIdsRef.size() * sizeof(uint32_t));

	// TODO Think how to pass data from entity context to this context more careful
	ArchetypeCreationContext archetypeContext;
	archetypeContext.add_components_id(componentIdsToMove);
	archetypeContext._idToSize = entityContext._typeIdToSize;
	archetypeContext._allComponentsSize = entityContext._allComponentsSize;
	
	ArchetypeHandle archHandle = create_archetype(archetypeContext);

	UUID newUUID = uuid ? uuid : UUID();
	Entity entity{ newUUID };
	
	Archetype& archetype = _archetypes[archHandle.get_id()];
	uint32_t entityColumn = archetype.add_entity(entity);
	EntityInArchetypeInfo entityInArchetype;
	entityInArchetype.column = entityColumn;
	entityInArchetype.archetypeId = archHandle.get_id();
	_entityToItsInfoInArchetype[entity] = entityInArchetype;

	archetype.set_components(entity, entityColumn, entityContext);

	return entity;
}

Entity EntitySystem::build_entity_from_json(UUID& uuid, std::string& json)
{
	nlohmann::json componentsData = nlohmann::json::parse(json);
	EntityCreationContext creationContext;
	
	for (auto& componentInfo : componentsData.items())
	{
		std::string componentName = componentInfo.key();
		uint32_t typeId = get_type_id_table()->get_type_id(componentName);
		factories::BaseFactory* factory = factories::get_table()->get_factory(typeId);
		factory->build(creationContext, typeId, componentName, componentsData);
	}

	Entity entity = create_entity(creationContext, uuid);
	return entity;
}

void EntitySystem::build_components_json_from_entity(Entity& entity, nlohmann::json& levelJson)
{
	EntityInArchetypeInfo entityInArchetype = _entityToItsInfoInArchetype[entity];
	Archetype& archetype = _archetypes[entityInArchetype.archetypeId];
	uint32_t column = entityInArchetype.column;
	uint32_t componentsNumberInArchetype = archetype._chunkStructure.componentIds.size();
	uint8_t* componentsArray = new uint8_t[constants::MAX_COMPONENT_COUNT * componentsNumberInArchetype];
	uint8_t* componentsArrayStartPtr = componentsArray;
	nlohmann::json componentsJson;
	
	for (int i = 0; i != componentsNumberInArchetype; ++i)
	{
		componentsArray += i * constants::MAX_COMPONENT_SIZE;
		uint32_t typeId = archetype._chunkStructure.componentIds[i];
		archetype.get_component_by_component_type_id(entity, column, typeId, componentsArray);
		serializers::BaseSerializer* serializer = serializers::get_table()->get_serializer(typeId);
		serializer->serialize(componentsArray, componentsJson);
	}
	
	levelJson[std::to_string(entity.get_uuid())] = componentsJson.dump();
	
	delete[] componentsArrayStartPtr;
}

void EntitySystem::destroy_entity(Entity& entity)
{
	auto entityIterator = _entityToItsInfoInArchetype.find(entity);
	if (entityIterator == _entityToItsInfoInArchetype.end())
		return;

	EntityInArchetypeInfo entityInArchetype = entityIterator->second;
	Archetype& archetype = _archetypes[entityInArchetype.archetypeId];
	archetype.destroy_entity(entity, entityInArchetype.column);
	_entityToItsInfoInArchetype.erase(entityIterator);
}
