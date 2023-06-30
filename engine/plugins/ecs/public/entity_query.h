#pragma once

#include "api.h"
#include "type_id_tables.h"

#include <vector>
#include <functional>
#include <unordered_map>

namespace ad_astris::ecs
{
	class Archetype;
	class ExecutionContext;
	
	enum class ComponentAccess : uint8_t
	{
		READ_ONLY = 1,
		READ_WRITE = 2
	};
	
	class ECS_API QueryRequirements
	{
		friend class SystemManager;
		
		public:
			template<typename T>
			void add_component_requirement(ComponentAccess componentAccess)
			{
				if (!ComponentTypeIDTable::check_in_table<T>())
					ComponentTypeIDTable::add_component_info<T>();
				
				uint32_t id = ComponentTypeIDTable::get_type_id<T>();
				_componentIDToAccess[id] = componentAccess;
				_requiredComponentIDs.push_back(id);
			}

			template<typename T>
			void add_tag_requirement()
			{
				if (!TagTypeIDTable::check_in_table<T>())
					TagTypeIDTable::add_tag<T>();
					
				_requiredTagIDs.push_back(TagTypeIDTable::get_type_id<T>());
			}

		protected:
			std::unordered_map<uint32_t, ComponentAccess> _componentIDToAccess;
			std::vector<uint32_t> _requiredComponentIDs;
			std::vector<uint32_t> _requiredTagIDs;
	};

	class ExecutionContext;
	
	class ECS_API EntityQuery : public QueryRequirements
	{
		friend class SystemManager;
		
		public:
			void for_each_chunk(std::function<void(ExecutionContext&)> executeFunction);
		
		private:
			std::vector<Archetype*> _archetypes;
			std::vector<ExecutionContext> _executionContexts;
	};
}