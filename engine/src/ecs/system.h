#pragma once

#include "entity_query.h"
#include "core/reflection.h"

#include <vector>
#include <string>

namespace ad_astris::ecs
{
	class SystemManager;
	
	class SystemExecutionOrder
	{
		friend SystemManager;
		
		public:
			template<typename T>
			void add_to_execute_before()
			{
				_executeBefore.push_back(get_type_name<T>());
			}

			template<typename T>
			void add_to_execute_after()
			{
				_executeAfter.push_back(get_type_name<T>());
			}

		private:
			std::vector<std::string> _executeBefore;
			std::vector<std::string> _executeAfter;
	};

	struct SystemRequirements
	{
		
	};
	
	class System
	{
		friend SystemManager;

		public:
			virtual void execute() = 0;
			virtual void configure_query() = 0;

		protected:
			SystemExecutionOrder _executionOrder;
			SystemRequirements _requirements;
			EntityQuery _entityQuery;
	};
}