#pragma once

#include "entity_manager.h"
#include "system.h"

namespace ad_astris::ecs
{
	// Static class
	class SystemManager
	{
		public:
			template<typename T>
			static void register_system()
			{
				
			}

			template<typename T>
			static T* get_system()
			{
				
			}

			static void execute();

			static void add_entity_manager(EntityManager* entityManager);

		private:
			static std::vector<EntityManager*> _entityManagers;
	};
}