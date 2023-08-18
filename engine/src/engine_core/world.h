#pragma once

#include "level/level.h"
#include "object.h"

#include <unordered_set>
#include <memory>
#include <ecs.h>

namespace ad_astris
{
	enum class EntityTemplate
	{
		PointLight,
		DirectionalLight,
		SpotLight,
		// TODO
	};
	
	struct Translation
	{
		float x;
		float y;
	};

	ECS_COMPONENT(Translation, float, float)

	struct Rotation
	{
		float x;
		float y;
		float z;
	};

	struct Enemy
	{
		
	};

	class PhysicsSystem : public ecs::System
	{
		public:
		PhysicsSystem()
		{

		}
		
		virtual void execute() override
		{
			auto func = [](ecs::ExecutionContext& executionContext){
				auto translations = executionContext.get_mutable_components<Translation>();
				auto rotations = executionContext.get_immutable_components<Rotation>();

				for (uint32_t i = 0; i != executionContext.get_entities_count(); ++i)
				{
					Translation translation = translations[i];
					translation.x += rotations[i].x * rotations[i].z;
					translation.y += rotations[i].y * rotations[i].z;
					translations.set(translation, i);
				}
			};
			_entityQuery.for_each_chunk(func);
		}

		virtual void configure_query() override
		{
			_entityQuery.add_component_requirement<Translation>(ecs::ComponentAccess::READ_WRITE);
			_entityQuery.add_component_requirement<Rotation>(ecs::ComponentAccess::READ_ONLY);
		}
	};

	class OutputSystem : public ecs::System
	{
		public:
		OutputSystem()
		{
			_executionOrder.add_to_execute_after<PhysicsSystem>();
		}
					
		virtual void execute() override
		{
			auto func = [](ecs::ExecutionContext& executionContext){
				auto translations = executionContext.get_immutable_components<Translation>();
				auto rotations = executionContext.get_immutable_components<Rotation>();

				for (uint32_t i = 0; i != executionContext.get_entities_count(); ++i)
				{
					Translation translation = translations[i];
					LOG_INFO("Translation: {} {}", translation.x, translation.y);
					Rotation rotation = rotations[i];
					LOG_INFO("Rotation: {} {} {}", rotation.x, rotation.y, rotation.z);
				}
			};

			_entityQuery.for_each_chunk(func);
		}

		virtual void configure_query() override
		{
			_entityQuery.add_component_requirement<Translation>(ecs::ComponentAccess::READ_ONLY);
			_entityQuery.add_component_requirement<Rotation>(ecs::ComponentAccess::READ_ONLY);
		}
	};
}
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

namespace ad_astris
{
	ECS_COMPONENT(Rotation, float, float, float)
	ECS_TAG(Enemy)
	ECS_SYSTEM(OutputSystem)
	ECS_SYSTEM(PhysicsSystem)
}