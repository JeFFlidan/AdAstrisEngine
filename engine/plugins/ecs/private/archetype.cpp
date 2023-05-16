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
		uint32_t subchunkSize = constants::MAX_ENTITIES_IN_CNUNK * structureSize;
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
	componentPtr = static_cast<uint8_t*>(component->get_raw_memory());
}

ecs::Subchunk ecs::ArchetypeChunk::get_subchunk(uint32_t componentTypeId)
{
	if (_componentIdToSubchunk.find(componentTypeId) == _componentIdToSubchunk.end())
	{
		LOG_ERROR("No component of this type")
		return Subchunk();
	}
	return _componentIdToSubchunk[componentTypeId];
}

uint8_t* ecs::ArchetypeChunk::get_entity_component(uint32_t column, uint32_t componentTypeId)
{
	Subchunk& subchunk = _componentIdToSubchunk[componentTypeId];
	return subchunk.get_ptr() + column * subchunk.get_structure_size(); 
}

ecs::Archetype::Archetype(ArchetypeCreationContext& context)
{
	_sizeOfOneColumn = context._allComponentsSize;
	_numEntitiesPerChunk = get_chunk_size() / context._allComponentsSize;
	_chunkStructure.componentIdToSize = std::move(context._idToSize);
	_chunkStructure.componentIds = std::move(context._ids);
}

uint32_t ecs::Archetype::add_entity(Entity& entity)
{
	ArchetypeChunk* requiredChunk = nullptr;
	uint16_t chunkIndex = 0;

	for (auto& chunk : _chunks)
	{
		if (chunk.get_elements_count() < _numEntitiesPerChunk)
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

	uint32_t rowIndex = requiredChunk->get_elements_count();
	requiredChunk->add_instance();
	_entityToChunk[entity] = chunkIndex;
	return rowIndex;
}

void ecs::Archetype::destroy_entity(uint32_t rowIndex)
{
	// TODO
}

uint32_t ecs::Archetype::get_chunk_size()
{
	return constants::MAX_ENTITIES_IN_CNUNK * _sizeOfOneColumn;
}

void ecs::Archetype::set_component(Entity& entity, uint32_t columnIndex, IComponent* tempComponent)
{
	
}

void ecs::Archetype::set_components(Entity& entity, uint32_t columnIndex, EntityCreationContext& creationContext)
{
	ArchetypeChunk& chunk = _chunks[_entityToChunk[entity]];
	for (auto& component : creationContext._componentsMap)
	{
		chunk.set_component(columnIndex, component.second);
	}
}
