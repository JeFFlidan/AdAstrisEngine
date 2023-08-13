#pragma once

#include "engine_core/object.h"
#include "level_common.h"
#include "ecs.h"

#include <vector>

namespace ad_astris::ecore
{
	class World;
	
	class Level : public Object
	{
		friend class level::Utils;
		
		// TODO
		public:
			Level() = default;
			// Constructor to create level with default settings
			Level(io::URI& path, ObjectName* levelName);

			World* get_owning_world();
			void set_owning_world(World* world);
			ecs::EntityManager* get_entity_manager();
			void build_entities();
			void add_entity(ecs::Entity& entity);

		private:
			World* _owningWorld{ nullptr }; // ?
			ecs::EntityManager* _entityManager{ nullptr };
			std::vector<ecs::Entity> _entities;
			
			level::LevelInfo _levelInfo;
			std::string _entitiesJsonStr;
		
		public:
			// ========== Begin Object interface ==========
				
			virtual void serialize(io::IFile* file) override;
			virtual void deserialize(io::IFile* file, ObjectName* objectName) override;
			virtual uint64_t get_size() override;
			virtual bool is_resource() override;
			virtual UUID get_uuid() override;
			virtual std::string get_description() override;
			virtual std::string get_type() override;
			
		protected:
			virtual void rename_in_engine(ObjectName& newName) override;

			// ========== End Object interface ==========
	};
}
