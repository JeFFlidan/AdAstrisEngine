#pragma once

#include "type_info_table.h"

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
	
	class QueryRequirements
	{
		friend class SystemManager;
		
		public:
			template<typename T>
			void add_component_requirement(ComponentAccess componentAccess)
			{
				if (!TypeInfoTable::check_component_in_table<T>())
					TypeInfoTable::add_component<T>();
				
				uint32_t id = TypeInfoTable::get_component_id<T>();
				_componentIDToAccess[id] = componentAccess;
				_requiredComponentIDs.push_back(id);
			}

			template<typename T>
			void add_tag_requirement()
			{
				if (!TypeInfoTable::check_tag_in_table<T>())
					TypeInfoTable::add_tag<T>();
					
				_requiredTagIDs.push_back(TypeInfoTable::get_tag_id<T>());
			}

		protected:
			std::unordered_map<uint32_t, ComponentAccess> _componentIDToAccess;
			std::vector<uint32_t> _requiredComponentIDs;
			std::vector<uint32_t> _requiredTagIDs;
	};

	class ExecutionContext;
	
	class EntityQuery : public QueryRequirements
	{
		friend class SystemManager;
		
		public:
			void for_each_chunk(std::function<void(ExecutionContext&)> executeFunction);
		
		private:
			std::vector<Archetype*> _archetypes;
			std::vector<ExecutionContext> _executionContexts;
	};
}
