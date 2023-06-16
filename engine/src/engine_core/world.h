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

	ECS_COMPONENT(Rotation, float, float, float)

	struct Enemy
	{
		
	};

	ECS_TAG(Enemy)
}

namespace ad_astris::ecore
{
	class World : public Object
	{
		public:
			void add_level(Level* level);

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
				ecs::EntityManager* manager = _currentLevel->get_entity_manager();
				ecs::EntityCreationContext creationContext;
				creationContext.add_component(translation);
				creationContext.add_component(rotation);
				LOG_INFO("Before adding tag")
				creationContext.add_tag<Enemy>();
				LOG_INFO("After adding tag")
				ecs::Entity entity = manager->create_entity(creationContext);
				LOG_INFO("After creation context")
				_currentLevel->add_entity(entity);
				LOG_INFO("After adding entity")

				Translation translation1 = manager->get_entity_component<Translation>(entity);
				Rotation rotation1 = manager->get_entity_component<Rotation>(entity);

				LOG_INFO("Translation: {} {}", translation1.x, translation1.y)
				LOG_INFO("Rotation: {} {} {}", rotation1.x, rotation1.y, rotation1.z)

				LOG_INFO("FINISH WORLD TEST")
			}
		
		private:
			std::unordered_set<Level*> _levels;
			Level* _currentLevel{ nullptr };
	};
}