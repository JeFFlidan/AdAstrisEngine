#pragma once

#include "basic_components.h"
#include "core/pool_allocator.h"
#include "events/event.h"

namespace ad_astris::ecore
{
#define IMPLEMENT_ENGINE_OBJECT_EVENT(EventName) \
	EVENT_TYPE_DECL(EventName) \
	EventName(ComponentContext* componentContext, ThreadSafePoolAllocator<ComponentContext>& poolAllocator) \
		: _componentContext(componentContext), _poolAllocator(poolAllocator) {} \
	virtual void cleanup() override \
	{\
		_poolAllocator.free(_componentContext);\
	}\
	ComponentContext* get_component_context()\
	{\
		return _componentContext;\
	} \
	private:\
		ComponentContext* _componentContext; \
		ThreadSafePoolAllocator<ComponentContext>& _poolAllocator;
	
	class StaticModelCreatedEvent : public events::IEvent
	{
		public:
			struct ComponentContext
			{
				ecs::Entity entity;
				TransformComponent transformComponent;
				ModelComponent modelComponent;
				CastShadowComponent castShadowComponent;
				VisibleComponent visibleComponent;
			};
		
			IMPLEMENT_ENGINE_OBJECT_EVENT(StaticModelCreatedEvent)
	};

	class SkeletalModelCreatedEvent : public events::IEvent
	{
		public:
			struct ComponentContext
			{
				ecs::Entity entity;
				TransformComponent transformComponent;
				ModelComponent modelComponent;
				CastShadowComponent castShadowComponent;
				VisibleComponent visibleComponent;
			};
			
			IMPLEMENT_ENGINE_OBJECT_EVENT(SkeletalModelCreatedEvent)
	};

	class PointLightCreatedEvent : public events::IEvent
	{
		public:
			struct ComponentContext
			{
				ecs::Entity entity;
				TransformComponent transformComponent;
				LuminanceIntensityComponent luminanceIntensityComponent;
				ColorComponent colorComponent;
				AttenuationRadiusComponent attenuationRadiusComponent;
				LightTemperatureComponent lightTemperatureComponent;
				CastShadowComponent castShadowComponent;
				VisibleComponent visibleComponent;
				AffectWorldComponent affectWorldComponent;
				ExtentComponent extentComponent;
			};
			
			IMPLEMENT_ENGINE_OBJECT_EVENT(PointLightCreatedEvent)
	};

	class DirectionalLightCreatedEvent : public events::IEvent
	{
		public:
			struct ComponentContext
			{
				ecs::Entity entity;
				TransformComponent transformComponent;
				CandelaIntensityComponent candelaIntensityComponent;
				ColorComponent colorComponent;
				LightTemperatureComponent lightTemperatureComponent;
				CastShadowComponent castShadowComponent;
				VisibleComponent visibleComponent;
				AffectWorldComponent affectWorldComponent;
				ExtentComponent extentComponent;
			};
		
			IMPLEMENT_ENGINE_OBJECT_EVENT(DirectionalLightCreatedEvent)
	};

	class SpotLightCreatedEvent : public events::IEvent
	{
		public:
			struct ComponentContext
			{
				ecs::Entity entity;
				TransformComponent transformComponent;
				LuminanceIntensityComponent luminanceIntensityComponent;
				ColorComponent colorComponent;
				AttenuationRadiusComponent attenuationRadiusComponent;
				LightTemperatureComponent lightTemperatureComponent;
				CastShadowComponent castShadowComponent;
				VisibleComponent visibleComponent;
				AffectWorldComponent affectWorldComponent;
				OuterConeAngleComponent outerConeAngleComponent;
				InnerConeAngleComponent innerConeAngleComponent;
				ExtentComponent extentComponent;
			};

			IMPLEMENT_ENGINE_OBJECT_EVENT(SpotLightCreatedEvent)
	};

	class CameraCreatedEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(CameraCreatedEvent)
			CameraCreatedEvent(ecs::Entity entity) : _entity(entity) { }

			ecs::Entity get_entity() { return _entity; }

		private:
			ecs::Entity _entity;
	};
}