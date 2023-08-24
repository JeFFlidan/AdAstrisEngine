#include "engine_objects_creator.h"

ad_astris::ecore::EngineObjectsCreator::EngineObjectsCreator(ecs::EntityManager* entityManager)
	: _entityManager(entityManager)
{
	setup_static_model_archetype();
	setup_skeletal_model_archetype();
	setup_point_light_archetype();
	setup_spot_light_archetype();
	setup_directional_light_archetype();
}

ad_astris::ecs::Entity ad_astris::ecore::EngineObjectsCreator::create_static_model(
	EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_model_components(entityCreationContext, objectCreationContext);
	entityCreationContext.add_tag<StaticObjectTag>();
	return _entityManager->create_entity(entityCreationContext);
}

ad_astris::ecs::Entity ad_astris::ecore::EngineObjectsCreator::create_skeletal_model(
	EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_model_components(entityCreationContext, objectCreationContext);
	entityCreationContext.add_tag<MovableObjectTag>();
	return _entityManager->create_entity(entityCreationContext);
}

ad_astris::ecs::Entity ad_astris::ecore::EngineObjectsCreator::create_point_light(
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
	return _entityManager->create_entity(entityCreationContext);
}

ad_astris::ecs::Entity ad_astris::ecore::EngineObjectsCreator::create_directional_light(
	EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_light_components(entityCreationContext, objectCreationContext);
	CandelaIntensityComponent intensityComponent;
	intensityComponent.intensity = 2.0f;
	entityCreationContext.add_component(intensityComponent);
	entityCreationContext.add_tag<StaticObjectTag>();
	entityCreationContext.add_tag<DirectionalLightTag>();
	return _entityManager->create_entity(entityCreationContext);
}

ad_astris::ecs::Entity ad_astris::ecore::EngineObjectsCreator::create_spot_light(
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
	return _entityManager->create_entity(entityCreationContext);
}

void ad_astris::ecore::EngineObjectsCreator::setup_static_model_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, ModelComponent, CastShadowComponent, VisibleComponent>();
	context.add_tags<StaticObjectTag>();
	_entityManager->create_archetype(context);
}

void ad_astris::ecore::EngineObjectsCreator::setup_skeletal_model_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, ModelComponent, CastShadowComponent, VisibleComponent>();
	context.add_tags<MovableObjectTag>();
	_entityManager->create_archetype(context);
}

void ad_astris::ecore::EngineObjectsCreator::setup_point_light_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, LuminanceIntensityComponent, ColorComponent, AttenuationRadiusComponent,
		UseLightTemperatureComponent, LightTemperatureComponent, CastShadowComponent, VisibleComponent, AffectWorldComponent>();
	context.add_tags<StaticObjectTag, PointLightTag>();
	_entityManager->create_archetype(context);
}

void ad_astris::ecore::EngineObjectsCreator::setup_directional_light_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, CandelaIntensityComponent, ColorComponent, UseLightTemperatureComponent,
		LightTemperatureComponent, CastShadowComponent, VisibleComponent, AffectWorldComponent>();
	context.add_tags<StaticObjectTag, DirectionalLightTag>();
	_entityManager->create_archetype(context);
}

void ad_astris::ecore::EngineObjectsCreator::setup_spot_light_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, LuminanceIntensityComponent, ColorComponent, AttenuationRadiusComponent,
		UseLightTemperatureComponent, LightTemperatureComponent, CastShadowComponent, VisibleComponent, AffectWorldComponent,
		OuterConeAngleComponent, InnerConeAngleComponent>();
	context.add_tags<StaticObjectTag, SpotLightTag>();
	_entityManager->create_archetype(context);
}

void ad_astris::ecore::EngineObjectsCreator::setup_basic_model_components(
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
	castShadowComponent.isShadowCast = true;
	entityCreationContext.add_component(castShadowComponent);
	VisibleComponent visibleComponent;
	visibleComponent.isVisible = true;
	entityCreationContext.add_component(visibleComponent);
}

void ad_astris::ecore::EngineObjectsCreator::setup_basic_light_components(
	ecs::EntityCreationContext& entityCreationContext,
	EditorObjectCreationContext& objectCreationContext)
{
	TransformComponent transformComponent{ };
	transformComponent.location = objectCreationContext.location;
	entityCreationContext.add_component(transformComponent);
	ColorComponent colorComponent;
	colorComponent.color = glm::vec4(1.0f);
	entityCreationContext.add_component(colorComponent);
	UseLightTemperatureComponent useLightTemperatureComponent;
	useLightTemperatureComponent.isTemperatureUsed = false;
	entityCreationContext.add_component(useLightTemperatureComponent);
	LightTemperatureComponent lightTemperatureComponent;
	lightTemperatureComponent.temperature = 6500.0f;
	entityCreationContext.add_component(lightTemperatureComponent);
	CastShadowComponent castShadowComponent;
	castShadowComponent.isShadowCast = true;
	entityCreationContext.add_component(castShadowComponent);
	VisibleComponent visibleComponent;
	visibleComponent.isVisible = true;
	entityCreationContext.add_component(visibleComponent);
	AffectWorldComponent affectWorldComponent;
	affectWorldComponent.isWorldAffected = true;
	entityCreationContext.add_component(affectWorldComponent);
}
