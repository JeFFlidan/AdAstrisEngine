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
			SystemManager();
			~SystemManager();
		
			void init();
			void cleanup();

			// Added new system to the SystemManager. New system should be inherited from the System class
			template<typename T>
			void register_system()
			{
				System* system = new T();
				system->configure_query();
				_idToSystem[SystemTypeIDTable::get_type_id<T>()] = system;
 			}

			// Returns a pointer to the existing system. You can get custom systems or engine default systems
			template<typename T>
			T* get_system()
			{
				return reinterpret_cast<T*>(_idToSystem[SystemTypeIDTable::get_type_id<T>()]);
			}

			/** Main purpose is changing default engine system to custom one. 
			 * @param oldSystemName must be a valid name of the old system. In most cases, it will be taken from
			 * ui
			 */
			template<typename T>
			void set_new_system(std::string oldSystemName)
			{
				// TODO I have to think about locks or something like that.
				// Maybe, it is a good idea to implement queue for updating for the next frame?
				// And should think about generating new execution order
				uint32_t oldSystemID = SystemTypeIDTable::get_type_id(oldSystemName);
				_idToSystem[oldSystemID] = new T();
			}

			// Executes all systems in right order. You can influence on the execution order
			// changing _executionOrder field in systems
			void execute();

			/** Adds new EntityManager. In most cases, EntityManager must be taken from the World object
			 * @param entityManager should be a valid pointer to the EntityManager
			 */
			void add_entity_manager(EntityManager* entityManager);

			// Generates new execution order based on the _executionOrder field from the System base class
			void generate_execution_order();

		private:
			static bool _isInitialized;
		
			std::vector<EntityManager*> _entityManagers;
			std::unordered_map<uint32_t, System*> _idToSystem;
			std::vector<uint32_t> _executionOrder;

			void update_queries(std::vector<EntityManager*>& managersForUpdate, System* system);
	};
}