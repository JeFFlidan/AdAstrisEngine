#pragma once

#include "archetype_types.h"
#include "entity_types.h"
#include "core/tuple.h"
#include <vector>

namespace ad_astris::ecs
{
	class EntitySystem;
	
	namespace constants
	{
		constexpr uint32_t ARCHETYPE_CHUNK_SIZE = 128 * 1024;
		constexpr uint32_t MAX_ENTITIES_IN_CNUNK = 1024;
	}
	
	struct ECS_API ChunkStructure
	{
		std::vector<uint32_t> componentIds;
		std::unordered_map<uint32_t, uint16_t> componentIdToSize;	// should be sorted
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
		friend EntitySystem;
		public:
			Archetype(ArchetypeCreationContext& context);

			/**
			 * 
			 * @return entity row index in the archetype*/
			uint32_t add_entity(Entity& entity);
			void destroy_entity(uint32_t rowIndex);

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
					((subchunks.push_back(chunk.get_subchunk(Component<TYPES>::_typeId))), ...);
				}

				return std::move(subchunks);
			}

			template<typename T>
			T* get_entity_component(Entity& entity, uint32_t columnIndex)
			{
				ArchetypeChunk& chunk = _chunks[_entityToChunk[entity]];
				return chunk.get_entity_component(columnIndex, Component<T>::_typeId);
			}

			template<typename ...ARGS>
			Tuple<ARGS*...> get_entity_components(Entity& entity, uint32_t columnIndex)
			{
				ArchetypeChunk& chunk = _chunks[_entityToChunk[entity]];
				Tuple<ARGS*...> tuple{ ((get_converted_component<ARGS>(chunk, columnIndex)), ...) };
				return std::move(tuple);
			}
		
			uint32_t get_chunk_size();
		
		private:
			std::unordered_map<Entity, uint16_t> _entityToChunk;
			std::vector<ArchetypeChunk> _chunks;

			ChunkStructure _chunkStructure;
		
			uint32_t _numEntitiesPerChunk{ 0 };
			uint32_t _sizeOfOneColumn{ 0 };

			template<typename T>
			T* get_converted_component(ArchetypeChunk& chunk, uint32_t columnIndex)
			{
				return reinterpret_cast<T*>(chunk.get_entity_component(columnIndex, Component<T>::_typeId));
			}
	};
}