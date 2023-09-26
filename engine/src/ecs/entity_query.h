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
				if (!TYPE_INFO_TABLE->check_component_in_table<T>())
					TYPE_INFO_TABLE->add_component_info<T>();
				
				uint32_t id = TYPE_INFO_TABLE->get_component_id<T>();
				_componentIDToAccess[id] = componentAccess;
				_requiredComponentIDs.push_back(id);
			}

			template<typename T>
			void add_tag_requirement()
			{
				if (!TYPE_INFO_TABLE->check_tag_in_table<T>())
					TYPE_INFO_TABLE->add_tag<T>();
					
				_requiredTagIDs.push_back(TYPE_INFO_TABLE->get_tag_id<T>());
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
