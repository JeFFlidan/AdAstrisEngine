#pragma once

#include "archetype.h"
#include "type_info_table.h"

#include "profiler/logger.h"
#include "core/reflection.h"
#include "core/array_view.h"

#include <vector>
#include <unordered_map>

namespace ad_astris::ecs
{
	enum class ComponentAccess : uint8_t
	{
		READ_ONLY = 1,
		READ_WRITE = 2
	};
	
	class ExecutionContext
	{
		public:
			ExecutionContext(Archetype* archetype, std::unordered_map<uint32_t, ComponentAccess>& accessByComponentID);
		
			template<typename T>
			ConstArrayView<T> get_immutable_components()
			{
				std::string componentName = get_type_name<T>();
				
				uint32_t id = TYPE_INFO_TABLE->get_component_id<T>();
				auto it = _accessByComponentID.find(id);
				if (it == _accessByComponentID.end())
				{
					LOG_FATAL("ExecutionContext::get_immutable_components(): No component with type {}", componentName)
				}
				
				ComponentAccess access = it->second;

				if (access != ComponentAccess::READ_ONLY)
				{
					LOG_FATAL("ExectutionContext::get_immutable_components(): {} component should be mutable", componentName);
				}

				load_subchunks<T>();
				Subchunk* subchunk = &_loadedSubchunks[id][_chunkIndex];

				uint32_t entitiesCount = _archetype->get_entities_count_per_chunk(_chunkIndex);
				ConstArrayView<T> componentsView(reinterpret_cast<T*>(subchunk->get_ptr()), entitiesCount);
				return componentsView;
			}

			template<typename T>
			ArrayView<T> get_mutable_components()
			{
				std::string componentName = get_type_name<T>();
				
				uint32_t id = TYPE_INFO_TABLE->get_component_id<T>();
				auto it = _accessByComponentID.find(id);
				if (it == _accessByComponentID.end())
				{
					LOG_FATAL("ExecutionContext::get_immutable_components(): No component with type {}", componentName);
				}
				
				ComponentAccess access = it->second;

				if (access != ComponentAccess::READ_WRITE)
				{
					LOG_FATAL("ExectutionContext::get_immutable_components(): {} component should be immutable", componentName);
				}

				load_subchunks<T>();
				Subchunk* subchunk = &_loadedSubchunks[id][_chunkIndex];

				uint32_t entitiesCount = _archetype->get_entities_count_per_chunk(_chunkIndex);
				ArrayView<T> componentsView(reinterpret_cast<T*>(subchunk->get_ptr()), entitiesCount);
				return componentsView;
			}
		
			void clear_loaded_subchunks();

			void set_chunk_index(uint32_t chunkIndex);
			uint32_t get_entities_count();

		private:
			std::unordered_map<uint32_t, ComponentAccess>& _accessByComponentID;
			std::unordered_map<uint32_t, std::vector<Subchunk>> _loadedSubchunks;
			Archetype* _archetype;
			uint32_t _chunkIndex{ 0 };

			template<typename T>
			void load_subchunks()
			{
				uint32_t typeID = TYPE_INFO_TABLE->get_component_id<T>();
				if (_loadedSubchunks.find(typeID) != _loadedSubchunks.end())
					return;

				_loadedSubchunks[typeID] = _archetype->get_subchunks_of_one_type<T>();
			}
	};
}