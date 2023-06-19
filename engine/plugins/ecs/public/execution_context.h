#pragma once

#include "api.h"
#include "entity_query.h"
#include "archetype.h"
#include "type_id_tables.h"

#include "profiler/logger.h"
#include "core/reflection.h"

#include <vector>
#include <unordered_map>

namespace ad_astris::ecs
{
	class ECS_API ExecutionContext
	{
		friend class SystemManager;
		
		public:
			ExecutionContext(Archetype* archetype);
		
			template<typename T>
			std::vector<T> get_immutable_components()
			{
				std::string componentName = get_type_name<T>();
				
				uint32_t id = ComponentTypeIDTable::get_type_id<T>();
				auto it = _componentIDToAccess.find(id);
				if (it == _componentIDToAccess.end())
				{
					LOG_ERROR("ExecutionContext::get_immutable_components(): No component with type {}", componentName);
					return std::vector<T>();
				}
				
				ComponentAccess access = it->second;

				if (access != ComponentAccess::READ_ONLY)
				{
					LOG_ERROR("ExectutionContext::get_immutable_components(): {} component should be mutable", componentName);
					return std::vector<T>();
				}

				load_subchunks<T>();
				Subchunk* subchunk = &_loadedSubchunks[id][_chunkIndex];

				uint32_t entitiesCount = _archetype->get_entities_count_per_chunk(_chunkIndex);
				std::vector<T> components(entitiesCount);
				memcpy(components.data(), subchunk->get_ptr(), entitiesCount * subchunk->get_structure_size());
				return components;
			}

			template<typename T>
			T* get_mutable_components()
			{
				std::string componentName = get_type_name<T>();
				
				uint32_t id = ComponentTypeIDTable::get_type_id<T>();
				auto it = _componentIDToAccess.find(id);
				if (it == _componentIDToAccess.end())
				{
					LOG_ERROR("ExecutionContext::get_immutable_components(): No component with type {}", componentName);
					return nullptr;
				}
				
				ComponentAccess access = it->second;

				if (access != ComponentAccess::READ_WRITE)
				{
					LOG_ERROR("ExectutionContext::get_immutable_components(): {} component should be immutable", componentName);
					return nullptr;
				}

				load_subchunks<T>();
				Subchunk* subchunk = &_loadedSubchunks[id][_chunkIndex];

				T* components = reinterpret_cast<T*>(subchunk->get_ptr());
				return components;
			}
		
			void clear_loaded_subchunks();

			void set_chunk_index(uint32_t chunkIndex);
			uint32_t get_entities_count();

		private:
			std::unordered_map<uint32_t, ComponentAccess> _componentIDToAccess;
			std::unordered_map<uint32_t, std::vector<Subchunk>> _loadedSubchunks;
			Archetype* _archetype;
			uint32_t _chunkIndex{ 0 };

			template<typename T>
			void load_subchunks()
			{
				uint32_t typeID = ComponentTypeIDTable::get_type_id<T>();
				if (_loadedSubchunks.find(typeID) != _loadedSubchunks.end())
					return;

				_loadedSubchunks[typeID] = _archetype->get_subchunks_of_one_type<T>();
			}
	};
}