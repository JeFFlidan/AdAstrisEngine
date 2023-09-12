#pragma once

#include "engine_core/contexts.h"
//#include "basic_components.h"
#include "engine_core/engine_object_events.h"
#include "core/pool_allocator.h"
#include "events/event_manager.h"

namespace ad_astris::engine::impl
{
	class EngineObjectsCreator
	{
		public:
			EngineObjectsCreator() = default;
			EngineObjectsCreator(ecs::EntityManager* entityManager, events::EventManager* eventManager);
		
			ecs::Entity create_static_model(ecore::EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_skeletal_model(ecore::EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_point_light(ecore::EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_directional_light(ecore::EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_spot_light(ecore::EditorObjectCreationContext& objectCreationContext);
		
		private:
			ecs::EntityManager* _entityManager{ nullptr };
			events::EventManager* _eventManager{ nullptr };
			ThreadSafePoolAllocator<ecore::StaticModelCreatedEvent::ComponentContext> _staticModelComponentContextPool;
			ThreadSafePoolAllocator<ecore::SkeletalModelCreatedEvent::ComponentContext> _skeletalModelComponentContextPool;
			ThreadSafePoolAllocator<ecore::PointLightCreatedEvent::ComponentContext> _pointLightComponentContextPool;
			ThreadSafePoolAllocator<ecore::DirectionalLightCreatedEvent::ComponentContext> _directionalLightComponentContextPool;
			ThreadSafePoolAllocator<ecore::SpotLightCreatedEvent::ComponentContext> _spotLightComponentContextPool;
		
			void setup_static_model_archetype();
			void setup_skeletal_model_archetype();
			void setup_point_light_archetype();
			void setup_directional_light_archetype();
			void setup_spot_light_archetype();
			void init_pool_allocators();
		
			void setup_basic_model_components(ecs::EntityCreationContext& entityCreationContext, ecore::EditorObjectCreationContext& objectCreationContext);
			void setup_basic_light_components(ecs::EntityCreationContext& entityCreationContext, ecore::EditorObjectCreationContext& objectCreationContext);
	};
}