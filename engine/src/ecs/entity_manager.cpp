#include "entity_manager.h"
#include "core/utils.h"

using namespace ad_astris::ecs;

constexpr uint32_t COMPONENT_SIZE = 2048;

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
	
	std::vector<uint64_t> newComponentTypeIDs;
	size_t componentsHash = merge_type_ids_vectors(
		newComponentTypeIDs,
		oldArchetypeChunkStructure.componentIds,
		context._componentIDs);

	std::vector<uint64_t> newTagTypeIDs;
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
	
	auto newComponentIdToSize = oldArchetype._chunkStructure.sizeByComponentID;
	newComponentIdToSize.insert(context._sizeByComponentID.begin(), context._sizeByComponentID.end());
	
	uint32_t newAllComponentsSize = 0;
	for (auto& idToSize : newComponentIdToSize)
	{
		newAllComponentsSize += idToSize.second;
	}
	
	ArchetypeCreationContext creationContext;
	creationContext._componentIDs = std::move(newComponentTypeIDs);
	creationContext._sizeByComponentID = std::move(newComponentIdToSize);
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
	std::vector<uint64_t> componentIdsToMove;
	copy_vector(entityContext._componentIDs, componentIdsToMove);

	std::vector<uint64_t> tagIDsToMove;
	copy_vector(entityContext._tagIDs, tagIDsToMove);
	
	ArchetypeCreationContext archetypeContext;
	archetypeContext._componentIDs = std::move(componentIdsToMove);
	archetypeContext._sizeByComponentID = entityContext._sizeByTypeID;
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

size_t get_entity_components_data_size(const ChunkStructure& chunkStructure)
{
	const size_t componentCount = chunkStructure.componentIds.size();
	return chunkStructure.sizeOfOneColumn + sizeof(uint32_t) * componentCount + sizeof(uint64_t) * componentCount;
}

constexpr const char* COMPONENT_COUNT_KEY = "component_count";
constexpr const char* COMPONENTS_DATA_OFFSET_KEY = "components_data_offset";
constexpr const char* TAG_IDS_KEY = "tag_ids";

Entity EntityManager::deserialize_entity(UUID& uuid, const nlohmann::json& entityJson, const std::vector<uint8_t>& componentsData)
{
	EntityCreationContext creationContext;
	const size_t componentCount = entityJson.at(COMPONENT_COUNT_KEY);

	size_t globalOffset = entityJson.at(COMPONENTS_DATA_OFFSET_KEY);
	const uint8_t* dataPtr = componentsData.data() + globalOffset;

	size_t offset = 0;
	for (size_t i = 0; i != componentCount; ++i)
	{
		uint64_t componentID = 0;
		memcpy(&componentID, dataPtr + offset, sizeof(uint64_t));
		offset += sizeof(uint64_t);
		uint32_t componentSize = 0;
		memcpy(&componentSize, dataPtr + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		creationContext.add_component(componentID, componentSize, dataPtr + offset);
		offset += componentSize;
	}

	creationContext._tagIDs = entityJson.at(TAG_IDS_KEY).get<std::vector<uint64_t>>();

	Entity entity = create_entity(creationContext, uuid);
	return entity;
}

void EntityManager::serialize_entity(Entity& entity, nlohmann::json& rootJson, std::vector<uint8_t>& componentsData)
{
	EntityInArchetypeInfo entityInArchetype = _entityToItsInfoInArchetype[entity];
	Archetype& archetype = _archetypes[entityInArchetype.archetypeId];
	const ChunkStructure& chunkStructure = archetype._chunkStructure;
	const uint32_t column = entityInArchetype.column;
	const size_t componentCount = chunkStructure.componentIds.size();
	const size_t globalOffset = componentsData.size();

	nlohmann::json entityJson;
	entityJson[COMPONENT_COUNT_KEY] = componentCount;
	entityJson[COMPONENTS_DATA_OFFSET_KEY] = globalOffset;

	componentsData.resize(componentsData.size() + get_entity_components_data_size(chunkStructure));
	uint8_t* dataPtr = componentsData.data() + globalOffset;
	
	size_t offset = 0;
	for (size_t i = 0; i != componentCount; ++i)
	{
		uint64_t componentID = chunkStructure.componentIds[i];
		memcpy(dataPtr + offset, &componentID, sizeof(uint64_t));
		offset += sizeof(uint64_t);
		auto it = chunkStructure.sizeByComponentID.find(componentID);
		assert(it != chunkStructure.sizeByComponentID.end());
		uint32_t componentSize = it->second;
		memcpy(dataPtr + offset, &componentSize, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		void* componentData = archetype.get_component_by_type_id(entity, column, componentID);
		memcpy(dataPtr + offset, componentData, componentSize);
		offset += componentSize;
	}
	
	entityJson[TAG_IDS_KEY] = chunkStructure.tagIDs;
	
	std::scoped_lock<std::mutex> locker(_entityMutex);
	rootJson[std::to_string(entity.get_uuid())] = entityJson;
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
	std::vector<uint64_t>& dstTypeIDs,
	std::vector<uint64_t>& srcTypeIDs,
	std::vector<uint64_t>& contextTypeIDs)
{
	dstTypeIDs.resize(srcTypeIDs.size() + contextTypeIDs.size());
	std::merge(contextTypeIDs.begin(), contextTypeIDs.end(), srcTypeIDs.begin(), srcTypeIDs.end(), dstTypeIDs.begin());
	auto last = std::unique(dstTypeIDs.begin(), dstTypeIDs.end());
	dstTypeIDs.erase(last);
	return CoreUtils::hash_numeric_vector(dstTypeIDs);
}

void EntityManager::copy_vector(
	std::vector<uint64_t>& srcTypeIDs,
	std::vector<uint64_t>& dstTypeIDs)
{
	if (!std::is_sorted(srcTypeIDs.begin(), srcTypeIDs.end()))
		std::sort(srcTypeIDs.begin(), srcTypeIDs.end());
	dstTypeIDs.resize(srcTypeIDs.size());
	memcpy(dstTypeIDs.data(), srcTypeIDs.data(), srcTypeIDs.size() * sizeof(uint64_t));
}

void EntityManager::get_entity_all_component_ids(Entity entity, std::vector<uint64_t>& ids)
{
	Archetype& archetype = _archetypes[_entityToItsInfoInArchetype[entity].archetypeId];
	ids = archetype._chunkStructure.componentIds;
}

void* EntityManager::get_entity_component_by_id(Entity entity, uint64_t id)
{
	Archetype& archetype = _archetypes[_entityToItsInfoInArchetype[entity].archetypeId];
	return archetype.get_component_by_type_id(entity, _entityToItsInfoInArchetype[entity].column, id);
}
