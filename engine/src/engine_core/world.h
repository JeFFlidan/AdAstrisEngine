#pragma once

#include "level/level.h"
#include "events/event_manager.h"
#include "multithreading/task_composer.h"
#include "object.h"

#include <unordered_set>
#include <memory>
#include "ecs/ecs.h"

namespace ad_astris::ecore
{
	struct WorldCreationContext
	{
		events::EventManager* eventManager{ nullptr };
		tasks::TaskComposer* taskComposer{ nullptr };
	};
	
	class World : public Object
	{
		public:
			World(WorldCreationContext& creationContext);

			ecs::EntityManager* get_entity_manager() { return _entityManager.get(); }
			events::EventManager* get_event_manager() { return _eventManager; }
			tasks::TaskComposer* get_task_composer() { return _taskComposer; }

			// Creates new entity at the current level
			ecs::Entity create_entity(ecs::EntityCreationContext& creationContext);
			// Adds an entity to the current level
			void add_entity(ecs::Entity& entity);
			void add_level(Level* level, bool isActive = false, bool isDefault = false);
			void set_current_level(Level* level);
			Level* get_current_level();
			void build_entities_for_active_levels();
		
		private:
			std::unique_ptr<ecs::EntityManager> _entityManager;
			
			std::unordered_set<Level*> _allLevels;
			std::unordered_set<Level*> _activeLevels;		// Have to think how to implement
			Level* _currentLevel{ nullptr };
			Level* _defaultLevel{ nullptr };
			events::EventManager* _eventManager{ nullptr };
			tasks::TaskComposer* _taskComposer{ nullptr };

		public:
			// ========== Begin Object interface ==========
			
			virtual void serialize(io::File* file) override;
			virtual void deserialize(io::File* file, ObjectName* newName = nullptr) override;

			// ========== End Object interface ==========
	};
}
