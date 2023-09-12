#include "engine_objects_creator.h"

using namespace ad_astris;
using namespace engine::impl;
using namespace ecore;

EngineObjectsCreator::EngineObjectsCreator(ecs::EntityManager* entityManager, events::EventManager* eventManager)
	: _entityManager(entityManager), _eventManager(eventManager)
{
	setup_static_model_archetype();
	setup_skeletal_model_archetype();
	setup_point_light_archetype();
	setup_spot_light_archetype();
	setup_directional_light_archetype();
	init_pool_allocators();
}

ecs::Entity EngineObjectsCreator::create_static_model(
	EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_model_components(entityCreationContext, objectCreationContext);
	entityCreationContext.add_tag<StaticObjectTag>();

	auto componentContext = _staticModelComponentContextPool.allocate();
	componentContext->modelComponent = entityCreationContext.get_component<ModelComponent>();
	componentContext->transformComponent = entityCreationContext.get_component<TransformComponent>();
	componentContext->visibleComponent = entityCreationContext.get_component<VisibleComponent>();
	componentContext->castShadowComponent = entityCreationContext.get_component<CastShadowComponent>();

	ecs::Entity entity = _entityManager->create_entity(entityCreationContext);
	componentContext->entity = entity;
	
	StaticModelCreatedEvent event(componentContext, _staticModelComponentContextPool);
	_eventManager->enqueue_event(event);
	
	return entity;
}

ecs::Entity EngineObjectsCreator::create_skeletal_model(
	EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_model_components(entityCreationContext, objectCreationContext);
	entityCreationContext.add_tag<MovableObjectTag>();

	auto componentContext = _skeletalModelComponentContextPool.allocate();
	componentContext->modelComponent = entityCreationContext.get_component<ModelComponent>();
	componentContext->transformComponent = entityCreationContext.get_component<TransformComponent>();
	componentContext->visibleComponent = entityCreationContext.get_component<VisibleComponent>();
	componentContext->castShadowComponent = entityCreationContext.get_component<CastShadowComponent>();

	ecs::Entity entity = _entityManager->create_entity(entityCreationContext);
	componentContext->entity = entity;
	
	SkeletalModelCreatedEvent event(componentContext, _skeletalModelComponentContextPool);
	_eventManager->enqueue_event(event);
	
	return entity;
}

ecs::Entity EngineObjectsCreator::create_point_light(
	EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_light_components(entityCreationContext, objectCreationContext);
	LuminanceIntensityComponent intensityComponent;
	intensityComponent.intensity = 1500.0f;
	entityCreationContext.add_component(intensityComponent);
	AttenuationRadiusComponent attenuationComponent;
	attenuationComponent.attenuationRadius = 1000.0f;
	entityCreationContext.add_component(attenuationComponent);
	entityCreationContext.add_tag<StaticObjectTag>();
	entityCreationContext.add_tag<PointLightTag>();

	auto componentContext = _pointLightComponentContextPool.allocate();
	componentContext->transformComponent = entityCreationContext.get_component<TransformComponent>();
	componentContext->luminanceIntensityComponent = entityCreationContext.get_component<LuminanceIntensityComponent>();
	componentContext->colorComponent = entityCreationContext.get_component<ColorComponent>();
	componentContext->attenuationRadiusComponent = entityCreationContext.get_component<AttenuationRadiusComponent>();
	componentContext->lightTemperatureComponent = entityCreationContext.get_component<LightTemperatureComponent>();
	componentContext->castShadowComponent = entityCreationContext.get_component<CastShadowComponent>();
	componentContext->visibleComponent = entityCreationContext.get_component<VisibleComponent>();
	componentContext->affectWorldComponent = entityCreationContext.get_component<AffectWorldComponent>();
	componentContext->extentComponent = entityCreationContext.get_component<ExtentComponent>();

	ecs::Entity entity = _entityManager->create_entity(entityCreationContext);
	componentContext->entity = entity;

	PointLightCreatedEvent event(componentContext, _pointLightComponentContextPool);
	_eventManager->enqueue_event(event);
	
	return entity;
}

ecs::Entity EngineObjectsCreator::create_directional_light(
	EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_light_components(entityCreationContext, objectCreationContext);
	CandelaIntensityComponent intensityComponent;
	intensityComponent.intensity = 2.0f;
	entityCreationContext.add_component(intensityComponent);
	entityCreationContext.add_tag<StaticObjectTag>();
	entityCreationContext.add_tag<DirectionalLightTag>();

	auto componentContext = _directionalLightComponentContextPool.allocate();
	componentContext->transformComponent = entityCreationContext.get_component<TransformComponent>();
	componentContext->candelaIntensityComponent = entityCreationContext.get_component<CandelaIntensityComponent>();
	componentContext->colorComponent = entityCreationContext.get_component<ColorComponent>();
	componentContext->lightTemperatureComponent = entityCreationContext.get_component<LightTemperatureComponent>();
	componentContext->castShadowComponent = entityCreationContext.get_component<CastShadowComponent>();
	componentContext->visibleComponent = entityCreationContext.get_component<VisibleComponent>();
	componentContext->affectWorldComponent = entityCreationContext.get_component<AffectWorldComponent>();
	componentContext->extentComponent = entityCreationContext.get_component<ExtentComponent>();

	ecs::Entity entity = _entityManager->create_entity(entityCreationContext);
	componentContext->entity = entity;

	DirectionalLightCreatedEvent event(componentContext, _directionalLightComponentContextPool);
	_eventManager->enqueue_event(event);
	
	return entity;
}

ecs::Entity EngineObjectsCreator::create_spot_light(
	EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_light_components(entityCreationContext, objectCreationContext);
	LuminanceIntensityComponent intensityComponent;
	intensityComponent.intensity = 1500.0f;
	entityCreationContext.add_component(intensityComponent);
	AttenuationRadiusComponent attenuationComponent;
	attenuationComponent.attenuationRadius = 1000.0f;
	entityCreationContext.add_component(attenuationComponent);
	OuterConeAngleComponent outerConeAngleComponent;
	outerConeAngleComponent.angle = 45.0f;
	entityCreationContext.add_component(outerConeAngleComponent);
	InnerConeAngleComponent innerConeAngleComponent;
	innerConeAngleComponent.angle = 0.0f;
	entityCreationContext.add_component(innerConeAngleComponent);
	entityCreationContext.add_tag<StaticObjectTag>();
	entityCreationContext.add_tag<SpotLightTag>();

	auto componentContext = _spotLightComponentContextPool.allocate();
	componentContext->transformComponent = entityCreationContext.get_component<TransformComponent>();
	componentContext->luminanceIntensityComponent = entityCreationContext.get_component<LuminanceIntensityComponent>();
	componentContext->colorComponent = entityCreationContext.get_component<ColorComponent>();
	componentContext->attenuationRadiusComponent = entityCreationContext.get_component<AttenuationRadiusComponent>();
	componentContext->lightTemperatureComponent = entityCreationContext.get_component<LightTemperatureComponent>();
	componentContext->castShadowComponent = entityCreationContext.get_component<CastShadowComponent>();
	componentContext->visibleComponent = entityCreationContext.get_component<VisibleComponent>();
	componentContext->affectWorldComponent = entityCreationContext.get_component<AffectWorldComponent>();
	componentContext->outerConeAngleComponent = entityCreationContext.get_component<OuterConeAngleComponent>();
	componentContext->innerConeAngleComponent = entityCreationContext.get_component<InnerConeAngleComponent>();
	componentContext->extentComponent = entityCreationContext.get_component<ExtentComponent>();

	ecs::Entity entity = _entityManager->create_entity(entityCreationContext);
	componentContext->entity = entity;

	SpotLightCreatedEvent event(componentContext, _spotLightComponentContextPool);
	_eventManager->enqueue_event(event);
	
	return entity;
}

void EngineObjectsCreator::setup_static_model_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, ModelComponent, CastShadowComponent, VisibleComponent>();
	context.add_tags<StaticObjectTag>();
	_entityManager->create_archetype(context);
}

void EngineObjectsCreator::setup_skeletal_model_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, ModelComponent, CastShadowComponent, VisibleComponent>();
	context.add_tags<MovableObjectTag>();
	_entityManager->create_archetype(context);
}

void EngineObjectsCreator::setup_point_light_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, LuminanceIntensityComponent, ColorComponent, AttenuationRadiusComponent,
		LightTemperatureComponent, CastShadowComponent, VisibleComponent, AffectWorldComponent, ExtentComponent>();
	context.add_tags<StaticObjectTag, PointLightTag>();
	_entityManager->create_archetype(context);
}

void EngineObjectsCreator::setup_directional_light_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, CandelaIntensityComponent, ColorComponent,
		LightTemperatureComponent, CastShadowComponent, VisibleComponent, AffectWorldComponent, ExtentComponent>();
	context.add_tags<StaticObjectTag, DirectionalLightTag>();
	_entityManager->create_archetype(context);
}

void EngineObjectsCreator::setup_spot_light_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, LuminanceIntensityComponent, ColorComponent, AttenuationRadiusComponent,
		LightTemperatureComponent, CastShadowComponent, VisibleComponent, AffectWorldComponent,
		OuterConeAngleComponent, InnerConeAngleComponent, ExtentComponent>();
	context.add_tags<StaticObjectTag, SpotLightTag>();
	_entityManager->create_archetype(context);
}

void EngineObjectsCreator::init_pool_allocators()
{
	_staticModelComponentContextPool.allocate_new_pool(10);
	_skeletalModelComponentContextPool.allocate_new_pool(10);
	_pointLightComponentContextPool.allocate_new_pool(10);
	_directionalLightComponentContextPool.allocate_new_pool(10);
	_spotLightComponentContextPool.allocate_new_pool(10);
}

void EngineObjectsCreator::setup_basic_model_components(
	ecs::EntityCreationContext& entityCreationContext,
	EditorObjectCreationContext& objectCreationContext)
{
	TransformComponent transformComponent{ };
	transformComponent.location = objectCreationContext.location;
	entityCreationContext.add_component(transformComponent);
	ModelComponent modelComponent;
	modelComponent.modelUUID = objectCreationContext.uuid;
	entityCreationContext.add_component(modelComponent);
	CastShadowComponent castShadowComponent;
	castShadowComponent.castShadows = true;
	entityCreationContext.add_component(castShadowComponent);
	VisibleComponent visibleComponent;
	visibleComponent.isVisible = true;
	entityCreationContext.add_component(visibleComponent);
}

void EngineObjectsCreator::setup_basic_light_components(
	ecs::EntityCreationContext& entityCreationContext,
	EditorObjectCreationContext& objectCreationContext)
{
	TransformComponent transformComponent{ };
	transformComponent.location = objectCreationContext.location;
	entityCreationContext.add_component(transformComponent);
	ColorComponent colorComponent;
	colorComponent.color = glm::vec4(1.0f);
	entityCreationContext.add_component(colorComponent);
	LightTemperatureComponent lightTemperatureComponent;
	lightTemperatureComponent.temperature = 6500.0f;
	lightTemperatureComponent.isTemperatureUsed = false;
	entityCreationContext.add_component(lightTemperatureComponent);
	CastShadowComponent castShadowComponent;
	castShadowComponent.castShadows = true;
	entityCreationContext.add_component(castShadowComponent);
	VisibleComponent visibleComponent;
	visibleComponent.isVisible = true;
	entityCreationContext.add_component(visibleComponent);
	AffectWorldComponent affectWorldComponent;
	affectWorldComponent.isWorldAffected = true;
	entityCreationContext.add_component(affectWorldComponent);
	ExtentComponent extentComponent;
	extentComponent.width = 2048;
	extentComponent.height = 2048;
	entityCreationContext.add_component(extentComponent);
}
