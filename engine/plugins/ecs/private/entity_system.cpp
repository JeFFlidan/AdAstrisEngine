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

ArchetypeHandle EntitySystem::get_entity_archetype(Entity& entity)
{
	return { _entityToArchetypeInfo[entity].archetypeId };
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
	EntityInArchetype entityInArchetype;
	entityInArchetype.column = columnIndex;
	entityInArchetype.archetypeId = archetypeHandle.get_id();
	_entityToArchetypeInfo[entity] = entityInArchetype;
	return entity;
}

Entity EntitySystem::create_entity(EntityCreationContext& entityContext)
{
	std::vector<uint32_t>& componentIdsRef = entityContext._componetsId;
	
	if (!std::is_sorted(componentIdsRef.begin(), componentIdsRef.end()))
		std::sort(componentIdsRef.begin(), componentIdsRef.end());
	std::vector<uint32_t> componentIdsToMove(componentIdsRef.size());
	memcpy(componentIdsToMove.data(), componentIdsRef.data(), componentIdsRef.size() * sizeof(uint32_t));
	
	ArchetypeCreationContext archetypeContext;
	archetypeContext.add_components_id(componentIdsToMove);
	ArchetypeHandle archHandle = create_archetype(archetypeContext);

	Entity entity{ UUID() };
	Archetype& archetype = _archetypes[archHandle.get_id()];
	uint32_t entityColumn = archetype.add_entity(entity);
	EntityInArchetype entityInArchetype;
	entityInArchetype.column = entityColumn;
	entityInArchetype.archetypeId = archHandle.get_id();
	_entityToArchetypeInfo[entity] = entityInArchetype;

	archetype.set_components(entity, entityColumn, entityContext);

	return entity;
}

void EntitySystem::set_up_component_common(Entity& entity, IComponent* component)
{
	EntityInArchetype& entityInArchetype = _entityToArchetypeInfo[entity];
	Archetype& archetype = _archetypes[entityInArchetype.archetypeId];
	archetype.set_component(entity, entityInArchetype.column, component);
}
