#pragma once

#include "engine_core/contexts.h"
#include "engine_core/world.h"

namespace ad_astris::engine::impl
{
	class EngineObjectsCreator
	{
		public:
			EngineObjectsCreator() = default;
			EngineObjectsCreator(ecore::World* world);
		
			ecs::Entity create_static_model(ecore::EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_skeletal_model(ecore::EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_point_light(ecore::EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_directional_light(ecore::EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_spot_light(ecore::EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_camera(ecore::EditorObjectCreationContext& objectCreationContext);
		
		private:
			ecore::World* _world{ nullptr };
		
			void setup_static_model_archetype();
			void setup_skeletal_model_archetype();
			void setup_point_light_archetype();
			void setup_directional_light_archetype();
			void setup_spot_light_archetype();
			void setup_camera_archetype();
		
			void setup_basic_model_components(ecs::EntityCreationContext& entityCreationContext, ecore::EditorObjectCreationContext& objectCreationContext);
			void setup_basic_light_components(ecs::EntityCreationContext& entityCreationContext, ecore::EditorObjectCreationContext& objectCreationContext);
	};
}