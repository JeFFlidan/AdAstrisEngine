#pragma once

#include "entity_manager.h"
#include "execution_context.h"
#include "system.h"
#include "events/event_manager.h"
#include "multithreading/task_composer.h"
#include "resource_manager/resource_manager.h"

namespace ad_astris::ecs
{
	struct EngineManagers
	{
		events::EventManager* eventManager{ nullptr };
		tasks::TaskComposer* taskComposer{ nullptr };
		resource::ResourceManager* resourceManager{ nullptr };
		EntityManager* entityManager{ nullptr };
	};
	
	// App can have only one instance of SystemManager. Otherwise there will be an assertion
	class SystemManager
	{
		public:
			SystemManager();
			~SystemManager();
		
			void init(EngineManagers& managers);
			void cleanup();

			// Added new system to the SystemManager. New system should be inherited from the System class
			template<typename T>
			void register_system()
			{
				TYPE_INFO_TABLE->add_system<T>();
				_systemByID[TYPE_INFO_TABLE->get_system_id<T>()] = std::make_unique<T>();
				_systemByID[TYPE_INFO_TABLE->get_system_id<T>()]->subscribe_to_events(_managers);
				_systemByID[TYPE_INFO_TABLE->get_system_id<T>()]->configure_execution_order();
				_systemByID[TYPE_INFO_TABLE->get_system_id<T>()]->configure_query();
 			}

			// Returns a pointer to the existing system. You can get custom systems or engine default systems
			template<typename T>
			T* get_system()
			{
				return reinterpret_cast<T*>(_systemByID[TYPE_INFO_TABLE->get_system_id<T>()]);
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
				uint32_t oldSystemID = TYPE_INFO_TABLE->get_system_id(oldSystemName);
				_systemByID[oldSystemID] = new T();
			}

			// Executes all systems in right order. You can influence on the execution order
			// changing _executionOrder field in systems
			void execute();

			/** Adds new EntityManager. In most cases, EntityManager must be taken from the World object
			 * @param entityManager should be a valid pointer to the EntityManager
			 */
			void add_entity_manager(EntityManager* entityManager);

			// Generates new execution order based on the _executionOrder field from the System base class.
			// First this method is called while launching the engine. While the engine is running, this method will only be called
			// if new systems have been added
			void generate_execution_order();

		private:
			static bool _isInitialized;
		
			std::vector<EntityManager*> _entityManagers;
			std::unordered_map<uint32_t, std::unique_ptr<System>> _systemByID;
			std::vector<uint32_t> _executionOrder;
			EngineManagers _managers;

			void update_queries(std::vector<EntityManager*>& managersForUpdate, System* system);
	};
}
