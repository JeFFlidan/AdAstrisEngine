#pragma once

#include "contexts.h"
#include "basic_components.h"

namespace ad_astris::ecore
{
	class EngineObjectsCreator
	{
		public:
			EngineObjectsCreator() = default;
			EngineObjectsCreator(ecs::EntityManager* entityManager);

			ecs::Entity create_static_model(EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_skeletal_model(EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_point_light(EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_directional_light(EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_spot_light(EditorObjectCreationContext& objectCreationContext);

		private:
			ecs::EntityManager* _entityManager{ nullptr };

			void setup_static_model_archetype();
			void setup_skeletal_model_archetype();
			void setup_point_light_archetype();
			void setup_directional_light_archetype();
			void setup_spot_light_archetype();

			void setup_basic_model_components(ecs::EntityCreationContext& entityCreationContext, EditorObjectCreationContext& objectCreationContext);
			void setup_basic_light_components(ecs::EntityCreationContext& entityCreationContext, EditorObjectCreationContext& objectCreationContext);
	};
}