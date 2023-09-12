#pragma once

#include "level/level.h"
#include "object.h"

#include <unordered_set>
#include <memory>
#include "ecs/ecs.h"

namespace ad_astris::ecore
{
	class World : public Object
	{
		public:
			void init();
			void cleanup();

			ecs::EntityManager* get_entity_manager();
				
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

		public:
			// ========== Begin Object interface ==========
			
			virtual void serialize(io::IFile* file) override;
			virtual void deserialize(io::IFile* file, ObjectName* newName = nullptr) override;

			// ========== End Object interface ==========
	};
}
