#pragma once

#include "level/level.h"
#include "object.h"
#include "profiler/logger.h"

#include <unordered_set>

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
					Translation* translations = executionContext.get_mutable_components<Translation>();
					auto rotations = executionContext.get_immutable_components<Rotation>();

					for (uint32_t i = 0; i != executionContext.get_entities_count(); ++i)
					{
						Translation& translation = translations[i];
						translation.x += rotations[i].x * rotations[i].z;
						translation.y += rotations[i].y * rotations[i].z;
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
						Translation& translation = translations[i];
						LOG_INFO("Translation: {} {}", translation.x, translation.y);
						Rotation& rotation = rotations[i];
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

	ECS_COMPONENT(Rotation, float, float, float)
	ECS_TAG(Enemy)
	
	ECS_SYSTEM(OutputSystem)
	ECS_SYSTEM(PhysicsSystem)
}

namespace ad_astris::ecore
{
	class World : public Object
	{
		public:
			void init();
			void cleanup();

			ecs::EntityManager* get_entity_manager();
				
			void add_level(Level* level);
			Level* get_current_level();

			void test_function()
			{
				LOG_INFO("WORLD START")
				Translation translation;
				translation.x = 10.5f;
				translation.y = 12.6f;
				Rotation rotation;
				rotation.x = 12.41f;
				rotation.y = 121.5f;
				rotation.z = 75.8f;
				ecs::EntityCreationContext creationContext;
				creationContext.add_component(translation);
				creationContext.add_component(rotation);
				LOG_INFO("Before adding tag")
				creationContext.add_tag<Enemy>();
				LOG_INFO("After adding tag")
				ecs::Entity entity = _entityManager.create_entity(creationContext);
				LOG_INFO("After creation context")
				_currentLevel->add_entity(entity);
				LOG_INFO("After adding entity")
					
				Translation translation1 = _entityManager.get_entity_component<Translation>(entity);
				Rotation rotation1 = _entityManager.get_entity_component<Rotation>(entity);
					
				LOG_INFO("Translation: {} {}", translation1.x, translation1.y)
				LOG_INFO("Rotation: {} {} {}", rotation1.x, rotation1.y, rotation1.z)

				LOG_INFO("FINISH WORLD TEST")
			}
		
		private:
			ecs::EntityManager _entityManager;
			
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