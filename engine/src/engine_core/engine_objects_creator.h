#pragma once

#include "contexts.h"
#include "basic_components.h"
#include "engine_object_events.h"
#include "core/pool_allocator.h"
#include "events/event_manager.h"

namespace ad_astris::ecore
{
	class EngineObjectsCreator
	{
		public:
			EngineObjectsCreator() = default;
			EngineObjectsCreator(ecs::EntityManager* entityManager, events::EventManager* eventManager);

			ecs::Entity create_static_model(EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_skeletal_model(EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_point_light(EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_directional_light(EditorObjectCreationContext& objectCreationContext);
			ecs::Entity create_spot_light(EditorObjectCreationContext& objectCreationContext);

		private:
			ecs::EntityManager* _entityManager{ nullptr };
			events::EventManager* _eventManager{ nullptr };
			ThreadSafePoolAllocator<StaticModelCreatedEvent::ComponentContext> _staticModelComponentContextPool;
			ThreadSafePoolAllocator<SkeletalModelCreatedEvent::ComponentContext> _skeletalModelComponentContextPool;
			ThreadSafePoolAllocator<PointLightCreatedEvent::ComponentContext> _pointLightComponentContextPool;
			ThreadSafePoolAllocator<DirectionalLightCreatedEvent::ComponentContext> _directionalLightComponentContextPool;
			ThreadSafePoolAllocator<SpotLightCreatedEvent::ComponentContext> _spotLightComponentContextPool;

			void setup_static_model_archetype();
			void setup_skeletal_model_archetype();
			void setup_point_light_archetype();
			void setup_directional_light_archetype();
			void setup_spot_light_archetype();
			void init_pool_allocators();

			void setup_basic_model_components(ecs::EntityCreationContext& entityCreationContext, EditorObjectCreationContext& objectCreationContext);
			void setup_basic_light_components(ecs::EntityCreationContext& entityCreationContext, EditorObjectCreationContext& objectCreationContext);
	};
}