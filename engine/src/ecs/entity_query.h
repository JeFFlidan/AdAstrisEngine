#pragma once

#include "execution_context.h"
#include "type_info_table.h"

#include <vector>
#include <functional>
#include <unordered_map>

namespace ad_astris::ecs
{
	class Archetype;
	class ExecutionContext;
	
	class QueryRequirements
	{
		friend class SystemManager;
		
		public:
			template<typename T>
			void add_component_requirement(ComponentAccess componentAccess)
			{
				uint64_t id = TypeInfoTable::get_component_id<T>();
				_componentIDToAccess[id] = componentAccess;
				_requiredComponentIDs.push_back(id);
			}

			template<typename T>
			void add_tag_requirement()
			{
				_requiredTagIDs.push_back(TypeInfoTable::get_tag_id<T>());
			}

		protected:
			std::unordered_map<uint64_t, ComponentAccess> _componentIDToAccess;
			std::vector<uint64_t> _requiredComponentIDs;
			std::vector<uint64_t> _requiredTagIDs;
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
