#pragma once

#include "archetype_types.h"
#include "entity_types.h"
#include "core/tuple.h"
#include <vector>

namespace ad_astris::ecs
{
	class EntityManager;
	
	namespace constants
	{
		constexpr uint32_t ARCHETYPE_CHUNK_SIZE = 128 * 1024;
		constexpr uint32_t MAX_ENTITIES_IN_CNUNK = 1024;
	}

	class ECS_API Subchunk
	{
		public:
			Subchunk() = default;
			Subchunk(uint8_t* startPtr, uint32_t subchunkSize, uint16_t structureSize)
				: _startPtr(startPtr), _subchunkSize(subchunkSize), _structureSize(structureSize) { }

			uint8_t* get_ptr() const
			{
				return _startPtr;
			}

			uint32_t get_subchunk_size() const
			{
				return _subchunkSize;
			}

			uint16_t get_structure_size() const
			{
				return _structureSize;
			}
		
		private:
			uint8_t* _startPtr{ nullptr };
			uint32_t _subchunkSize{ 0 };
			uint16_t _structureSize{ 0 };
	};
	
	struct ECS_API ChunkStructure
	{
		std::vector<uint32_t> componentIds;
		std::unordered_map<uint32_t, uint16_t> componentIdToSize;	// should be sorted
		std::vector<uint32_t> tagIDs;
		uint32_t entityCount;
	};
	
	class ECS_API ArchetypeChunk
	{
		public:
			ArchetypeChunk(uint32_t chunkSize, ChunkStructure& chunkStructure);
			~ArchetypeChunk();
		
			void add_several_instances(uint32_t count);
			void remove_several_instances(uint32_t count);

			void add_instance();
			void remove_instance();
		
			uint32_t get_elements_count();
			uint8_t* get_chunk();
		
			void set_component(uint32_t column, IComponent* component);
			Subchunk get_subchunk(uint32_t componentTypeId);
			uint8_t* get_entity_component(uint32_t column, uint32_t componentTypeId);

		private:
			uint8_t* _chunk{ nullptr };
			std::unordered_map<uint32_t, Subchunk> _componentIdToSubchunk;
			uint32_t _chunkSize{ 0 };
			uint32_t _elementsCount{ 0 };
	};

	class ECS_API Archetype
	{
		friend EntityManager;
		
		public:
			Archetype(ArchetypeCreationContext& context);

			/**
			 * 
			 * @return entity row index in the archetype*/
			uint32_t add_entity(Entity& entity);
			void destroy_entity(Entity& entity, uint32_t rowIndex);

			void set_component(Entity& entity, uint32_t columnIndex, IComponent* tempComponent);
			void set_components(Entity& entity, uint32_t columnIndex, EntityCreationContext& creationContext);

			/**
			 * @warning THINK ABOUT TUPLES!!!!!
			 */
			template<typename ...TYPES>
			std::vector<Subchunk> get_subchunks()
			{
				std::vector<Subchunk> subchunks;
				for (auto& chunk : _chunks)
				{
					((subchunks.push_back(chunk.get_subchunk(TypeInfoTable::get_component_id<TYPES>()))), ...);
				}

				return subchunks;
			}

			template<typename T>
			std::vector<Subchunk> get_subchunks_of_one_type()
			{
				std::vector<Subchunk> subchunks;
				for (auto& chunk : _chunks)
				{
					subchunks.push_back(chunk.get_subchunk(TypeInfoTable::get_component_id<T>()));
				}

				return subchunks;
			}

			template<typename T>
			T* get_entity_component(Entity& entity, uint32_t columnIndex)
			{
				ArchetypeChunk& chunk = _chunks[_entityToChunk[entity]];
				return reinterpret_cast<T*>(chunk.get_entity_component(columnIndex, TypeInfoTable::get_component_id<T>()));
			}

			template<typename ...ARGS>
			Tuple<ARGS*...> get_entity_components(Entity& entity, uint32_t columnIndex)
			{
				ArchetypeChunk& chunk = _chunks[_entityToChunk[entity]];
				Tuple<ARGS*...> tuple{ get_converted_component<ARGS>(chunk, columnIndex)... };
				return tuple;
			}

			template<typename ...ARGS>
			std::vector<Tuple<ARGS...>> get_component_arrays()
			{
				// TODO
			}
		
			uint32_t get_chunk_size();
			uint32_t get_chunks_count();

			uint32_t get_entities_count_per_chunk(uint32_t chunkIndex);

			bool check_requirements_match(
				std::vector<uint32_t>& requiredComponentIDs,
				std::vector<uint32_t>& requiredTagIDs);
		
		private:
			std::unordered_map<Entity, uint16_t> _entityToChunk;
			std::vector<ArchetypeChunk> _chunks;
			std::vector<uint32_t> _freeColumns;

			ChunkStructure _chunkStructure;
		
			uint32_t _numEntitiesPerChunk{ 0 };
			uint32_t _sizeOfOneColumn{ 0 };

			// I use this method for serialization
			void get_component_by_component_type_id(
				Entity& entity,
				uint32_t columnIndex,
				uint32_t typeId,
				uint8_t* tempComponentsArray);
		
			template<typename T>
			T* get_converted_component(ArchetypeChunk& chunk, uint32_t columnIndex)
			{
				return reinterpret_cast<T*>(chunk.get_entity_component(columnIndex, TypeInfoTable::get_component_id<T>()));
			}
	};
}