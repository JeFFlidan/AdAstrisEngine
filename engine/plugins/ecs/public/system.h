#pragma once

#include "api.h"

#include <vector>

namespace ad_astris::ecs
{
	class SystemManager;
	
	class ECS_API SystemExecutionOrder
	{
		friend SystemManager;
		
		public:
			template<typename T>
			void add_to_execute_before()
			{
				
			}

			template<typename T>
			void add_to_execute_after()
			{
				
			}

		private:
			std::vector<uint32_t> _executeBefore;
			std::vector<uint32_t> _executeAfter;
	};

	struct ECS_API SystemRequirements
	{
		
	};
	
	class ECS_API ISystem
	{
		friend SystemManager;

		public:
			virtual void update() = 0;

		protected:
			SystemExecutionOrder _executionOrder;
			SystemRequirements _requirements;
	};
}