#pragma once

#include "api.h"
#include "entity_types.h"
#include <cstdint>
#include <algorithm>
#include <functional>

namespace ad_astris::ecs
{
	class EntitySystem;
	class Archetype;
	
	class ECS_API ArchetypeHandle
	{
		ECS_API friend class EntitySystem;
		
		public:
			ArchetypeHandle() = default;
			uint32_t get_id()
			{
				return _id;
			}

		private:
			ArchetypeHandle(uint32_t id) : _id(id) {}
			uint32_t _id;
	};

	// Can be passed into EntitySystem only once because all data will be moved 
	class ECS_API ArchetypeCreationContext
	{
		ECS_API friend Archetype;
		ECS_API friend EntitySystem;
		
		public:
			template<typename ...TYPES>
			void add_components_id()
			{
				((set_up<TYPES>()), ...);
				std::sort(_ids.begin(), _ids.end());
			}

			void add_components_id(std::vector<uint32_t>& ids)
			{
				// TODO Should improve
				_ids = std::move(ids);
			}

		
		protected:
			std::unordered_map<uint32_t, uint16_t> _idToSize;
			uint32_t _allComponentsSize{ 0 };
			std::vector<uint32_t> _ids;

			template<typename T>
			void set_up()
			{
				// if (!get_type_id_table()->check_in_table<T>())
				// {
				// 	get_type_id_table()->set_component_info<T>();
				// }
				uint32_t id = get_type_id_table()->get_type_id<T>();
				uint32_t size = get_type_id_table()->get_type_size<T>();
				_allComponentsSize += size;
				_idToSize[id] = size;
				_ids.push_back(id);
				LOG_INFO("Context. Id back: {}", _ids.back())
			}
	};

	class ECS_API ArchetypeExtensionContext : public ArchetypeCreationContext
	{
		friend EntitySystem;
		
		public:
			ArchetypeExtensionContext(ArchetypeHandle srcArchetype) : _srcArchetype(srcArchetype)
			{
				
			}

		protected:
			ArchetypeHandle _srcArchetype;
	};

	class ECS_API ArchetypeReductionContext : public ArchetypeExtensionContext
	{
		public:
			ArchetypeReductionContext(ArchetypeHandle srcArchetype) : ArchetypeExtensionContext(srcArchetype)
			{
				
			}
	};
}
