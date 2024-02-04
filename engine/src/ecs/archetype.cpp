#include "archetype.h"
#include "profiler/logger.h"

#include <cstdlib>

using namespace ad_astris;

ecs::ArchetypeChunk::ArchetypeChunk(uint32_t chunkSize, ChunkStructure& chunkStructure) : _chunkSize(chunkSize)
{
	_chunk = static_cast<uint8_t*>(std::malloc(chunkSize));

	uint32_t prevSubchunkSizes = 0;
	for (auto& id : chunkStructure.componentIds)
	{
		uint16_t structureSize = chunkStructure.componentIdToSize[id];
		uint32_t subchunkSize = chunkStructure.numEntitiesPerChunk * structureSize;
		uint8_t* startPtr = _chunk + prevSubchunkSizes;
		Subchunk subchunk(startPtr, subchunkSize, structureSize);
		_componentIdToSubchunk[id] = subchunk;

		prevSubchunkSizes += subchunkSize;
	}
}

ecs::ArchetypeChunk::~ArchetypeChunk()
{
	if (_chunkSize)
		std::free(_chunk);
}

void ecs::ArchetypeChunk::add_several_instances(uint32_t count)
{
	_elementsCount += count;
}

void ecs::ArchetypeChunk::remove_several_instances(uint32_t count)
{
	if (_elementsCount - count <= 0)
	{
		_elementsCount = 0;
		std::free(_chunk);
	}
	else
	{
		_elementsCount -= count;
	}
}

void ecs::ArchetypeChunk::add_instance()
{
	add_several_instances(1);
}

void ecs::ArchetypeChunk::remove_instance()
{
	remove_several_instances(1);
}

uint32_t ecs::ArchetypeChunk::get_elements_count()
{
	return _elementsCount;
}

uint8_t* ecs::ArchetypeChunk::get_chunk()
{
	return _chunk;
}

void ecs::ArchetypeChunk::set_component(uint32_t column, IComponent* component)
{
	uint8_t* componentPtr = get_entity_component(column, component->get_type_id());
	uint8_t* tempPtr = static_cast<uint8_t*>(component->get_raw_memory());
	memcpy(componentPtr, tempPtr, component->get_structure_size());
}

ecs::Subchunk ecs::ArchetypeChunk::get_subchunk(uint64_t componentTypeId)
{
	if (_componentIdToSubchunk.find(componentTypeId) == _componentIdToSubchunk.end())
	{
		LOG_ERROR("No component of this type")
		return Subchunk();
	}
	return _componentIdToSubchunk[componentTypeId];
}

uint8_t* ecs::ArchetypeChunk::get_entity_component(uint32_t column, uint64_t componentTypeId)
{
	Subchunk& subchunk = _componentIdToSubchunk[componentTypeId];
	return subchunk.get_ptr() + column * subchunk.get_structure_size(); 
}

ecs::Archetype::Archetype(ArchetypeCreationContext& context)
{
	_chunkStructure.sizeOfOneColumn = context._allComponentsSize;
	_chunkStructure.numEntitiesPerChunk = context._entityCount;
	_chunkStructure.componentIds = std::move(context._componentIDs);
	_chunkStructure.componentIdToSize = std::move(context._idToSize);
	_chunkStructure.tagIDs = std::move(context._tagIDs);
	for (auto& tagID : _chunkStructure.tagIDs)
		_chunkStructure.tagIDsSet.insert(tagID);
}

uint32_t ecs::Archetype::add_entity(Entity& entity)
{
	ArchetypeChunk* requiredChunk = nullptr;
	uint16_t chunkIndex = 0;

	for (auto& chunk : _chunks)
	{
		if (chunk.get_elements_count() < _chunkStructure.numEntitiesPerChunk)
		{
			requiredChunk = &chunk;
			break;
		}
		++chunkIndex;
	}

	if (!requiredChunk)
	{
		_chunks.emplace_back(get_chunk_size(), _chunkStructure);
		requiredChunk = &_chunks.back();
		if (_chunks.size() > 1)
			chunkIndex = _chunks.size() - 1;
	}

	uint32_t columnIndex;
	if (!_freeColumns.empty())
	{
		columnIndex = _freeColumns.back();
		_freeColumns.pop_back();
	}
	else
	{
		columnIndex = requiredChunk->get_elements_count();
	}
	
	requiredChunk->add_instance();
	_entityToChunk[entity] = chunkIndex;
	return columnIndex;
}

void ecs::Archetype::destroy_entity(Entity& entity, uint32_t rowIndex)
{
	auto entityIterator = _entityToChunk.find(entity);
	if (entityIterator == _entityToChunk.end())
		return;
	_entityToChunk.erase(entityIterator);
	_freeColumns.push_back(rowIndex);
}

uint32_t ecs::Archetype::get_chunk_size()
{
	return _chunkStructure.numEntitiesPerChunk * _chunkStructure.sizeOfOneColumn;
}

uint32_t ecs::Archetype::get_chunks_count()
{
	return _chunks.size();
}

uint32_t ecs::Archetype::get_entities_count_per_chunk(uint32_t chunkIndex)
{
	return _chunks[chunkIndex].get_elements_count();
}

bool ecs::Archetype::check_requirements_match(
	std::vector<uint64_t>& requiredComponentIDs,
	std::vector<uint64_t>& requiredTagIDs)
{
	bool isMatched = false;
	if (!requiredComponentIDs.empty())
	{
		if (!std::is_sorted(requiredComponentIDs.begin(), requiredComponentIDs.end()))
		{
			std::sort(requiredComponentIDs.begin(), requiredComponentIDs.end());
		}

		std::vector<uint64_t>& archetypeComponents = _chunkStructure.componentIds;
		isMatched = std::includes(
			archetypeComponents.begin(),
			archetypeComponents.end(),
			requiredComponentIDs.begin(),
			requiredComponentIDs.end());
	}

	if (!requiredTagIDs.empty())
	{
		if (!std::is_sorted(requiredTagIDs.begin(), requiredTagIDs.end()))
		{
			std::sort(requiredTagIDs.begin(), requiredTagIDs.end());
		}

		std::vector<uint64_t>& archetypeTags = _chunkStructure.tagIDs;
		isMatched &= std::includes(
			archetypeTags.begin(),
			archetypeTags.end(),
			requiredTagIDs.begin(),
			requiredTagIDs.end());
	}

	return isMatched;
}

void ecs::Archetype::set_component(Entity& entity, uint32_t columnIndex, IComponent* tempComponent)
{
	ArchetypeChunk& chunk = _chunks[_entityToChunk[entity]];
	chunk.set_component(columnIndex, tempComponent);
}

void ecs::Archetype::set_components(Entity& entity, uint32_t columnIndex, EntityCreationContext& creationContext)
{
	ArchetypeChunk& chunk = _chunks[_entityToChunk[entity]];
	for (auto& component : creationContext._componentsMap)
	{
		chunk.set_component(columnIndex, component.second.get());
	}
}

void ecs::Archetype::get_component_by_component_type_id(
	Entity& entity,
	uint32_t columnIndex,
	uint64_t typeId,
	uint8_t* tempComponentsArray)
{
	ArchetypeChunk& chunk = _chunks[_entityToChunk[entity]];
	uint32_t componentSize = _chunkStructure.componentIdToSize[typeId];
	void* component = chunk.get_entity_component(columnIndex, typeId);
	memcpy(tempComponentsArray, component, componentSize);
}

void* ecs::Archetype::get_component_by_component_type_id(Entity& entity, uint32_t columnIndex, uint64_t typeID)
{
	ArchetypeChunk& chunk = _chunks[_entityToChunk[entity]];
	uint32_t componentSize = _chunkStructure.componentIdToSize[typeID];
	return chunk.get_entity_component(columnIndex, typeID);
}
