#pragma once

#include "entity_manager.h"
#include "execution_context.h"
#include "system.h"

namespace ad_astris::ecs
{
	// Static class
	class ECS_API SystemManager
	{
		public:
			static void init();
			static void cleanup();
		
			template<typename T>
			static void register_system()
			{
				System* system = new T();
				system->configure_query();
				_idToSystem[SystemTypeIDTable::get_type_id<T>()] = system;
 			}

			template<typename T>
			static T* get_system()
			{
				return reinterpret_cast<T*>(_idToSystem[SystemTypeIDTable::get_type_id<T>()]);
			}

			template<typename T>
			static void set_new_system(std::string oldSystemName)
			{
				// TODO I have to think about locks or something like that.
				// Maybe, it is a good idea to implement queue for updating for the next frame?
				// And should think about generating new execution order
				uint32_t oldSystemID = SystemTypeIDTable::get_type_id(oldSystemName);
				_idToSystem[oldSystemID] = new T();
			}

			static void execute();

			static void add_entity_manager(EntityManager* entityManager);

			static void generate_execution_order();

		private:
			static std::vector<EntityManager*> _entityManagers;
			static std::unordered_map<uint32_t, System*> _idToSystem;
			static std::vector<uint32_t> _executionOrder;

			static void update_queries(std::vector<EntityManager*>& managersForUpdate, System* system);
	};
}