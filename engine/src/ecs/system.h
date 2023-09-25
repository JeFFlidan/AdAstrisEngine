#pragma once

#include "entity_query.h"
#include "core/reflection.h"
#include "multithreading/task_composer.h"

#include <vector>
#include <string>

namespace ad_astris::ecs
{
	class SystemManager;
	struct EngineManagers;
	
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
			virtual ~System() { }
			virtual void subscribe_to_events(EngineManagers& managers) = 0;
			virtual void configure_query() = 0;
			virtual void configure_execution_order() = 0;
			virtual void execute(EngineManagers& managers, tasks::TaskGroup& globalTaskGroup) = 0;

		protected:
			SystemExecutionOrder _executionOrder;
			SystemRequirements _requirements;
			EntityQuery _entityQuery;
	};
}

#define OVERRIDE_SYSTEM_METHODS()															\
	virtual void subscribe_to_events(ad_astris::ecs::EngineManagers& managers) override;	\
	virtual void configure_query() override;												\
	virtual void configure_execution_order() override;										\
	virtual void execute(ad_astris::ecs::EngineManagers& managers, ad_astris::tasks::TaskGroup& globalTaskGroup) override;
