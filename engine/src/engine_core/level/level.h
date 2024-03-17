#pragma once

#include "engine_core/object.h"
#include "level_common.h"
#include "ecs/ecs.h"

#include <vector>

namespace ad_astris::ecore
{
	class World;

	struct LevelInfo
	{

	};
	
	struct LevelCreateInfo
	{
		io::URI path;
		std::string name;
	};
	
	class Level : public Object
	{
		friend class level::Utils;
		
		// TODO
		public:
			Level();
			// Constructor to create level with default settings
			Level(io::URI& path, ObjectName* levelName);
			Level(const LevelInfo& info, ObjectName* name);

			World* get_owning_world();
			void set_owning_world(World* world);
			ecs::EntityManager* get_entity_manager();
			void build_entities();
			void add_entity(ecs::Entity& entity);
			ecs::Entity create_entity(ecs::EntityCreationContext& creationContext);

		private:
			World* _owningWorld{ nullptr }; // ?
			ecs::EntityManager* _entityManager{ nullptr };
			std::vector<ecs::Entity> _entities;
			
			level::LevelInfo _levelInfo;		// temp
			LevelInfo _info;
			nlohmann::json _entitiesJson;
		
		public:
			// ========== Begin Object interface ==========
				
			virtual void serialize(io::File* file) override;
			virtual void deserialize(io::File* file, ObjectName* objectName) override;
			virtual uint64_t get_size() override;
			virtual bool is_resource() override;
			virtual UUID get_uuid() override;
			virtual std::string get_description() override;
			virtual std::string get_type() override;
			virtual void accept(resource::IResourceVisitor& resourceVisitor) override;

			// ========== End Object interface ==========
	};
}
