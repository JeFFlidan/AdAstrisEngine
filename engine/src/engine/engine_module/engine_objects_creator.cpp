#include "engine_objects_creator.h"
#include "engine_core/basic_components.h"

using namespace ad_astris;
using namespace engine::impl;
using namespace ecore;

EngineObjectsCreator::EngineObjectsCreator(World* world) : _world(world)
{
	setup_static_model_archetype();
	setup_skeletal_model_archetype();
	setup_point_light_archetype();
	setup_spot_light_archetype();
	setup_directional_light_archetype();
}

ecs::Entity EngineObjectsCreator::create_static_model(
	EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_model_components(entityCreationContext, objectCreationContext);
	entityCreationContext.add_tag<StaticObjectTag>();
	
	return _world->create_entity(entityCreationContext);
}

ecs::Entity EngineObjectsCreator::create_skeletal_model(
	EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_model_components(entityCreationContext, objectCreationContext);
	entityCreationContext.add_tag<MovableObjectTag>();

	return _world->create_entity(entityCreationContext);
}

ecs::Entity EngineObjectsCreator::create_point_light(
	EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_light_components(entityCreationContext, objectCreationContext);
	entityCreationContext.add_component<IntensityComponent>(8.0f);
	entityCreationContext.add_component<AttenuationRadiusComponent>(1000.0f);
	entityCreationContext.add_tag<StaticObjectTag>();
	entityCreationContext.add_tag<PointLightTag>();

	return _world->create_entity(entityCreationContext);
}

ecs::Entity EngineObjectsCreator::create_directional_light(
	EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_light_components(entityCreationContext, objectCreationContext);
	entityCreationContext.add_component<IntensityComponent>(1.0f);
	entityCreationContext.add_tag<StaticObjectTag>();
	entityCreationContext.add_tag<DirectionalLightTag>();

	return _world->create_entity(entityCreationContext);
}

ecs::Entity EngineObjectsCreator::create_spot_light(
	EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_light_components(entityCreationContext, objectCreationContext);
	entityCreationContext.add_component<IntensityComponent>(8.0f);
	entityCreationContext.add_component<AttenuationRadiusComponent>(1000.0f);
	entityCreationContext.add_component<OuterConeAngleComponent>(45.0f);
	entityCreationContext.add_component<InnerConeAngleComponent>(0.0f);
	entityCreationContext.add_tag<StaticObjectTag>();
	entityCreationContext.add_tag<SpotLightTag>();

	return _world->create_entity(entityCreationContext);
}

ecs::Entity EngineObjectsCreator::create_camera(EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	CameraComponent cameraComponent{};
	cameraComponent.movementSpeed = 30.0f;   
	cameraComponent.mouseSensitivity = 0.1f;
	entityCreationContext.add_component(cameraComponent);

	TransformComponent transformComponent;
	transformComponent.location = objectCreationContext.location;
	transformComponent.rotation = objectCreationContext.rotation;
	transformComponent.scale = objectCreationContext.scale;
	entityCreationContext.add_component(transformComponent);

	return _world->create_entity(entityCreationContext);
}

void EngineObjectsCreator::setup_static_model_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, ModelComponent, CastShadowComponent, VisibleComponent, OpaquePBRMaterialComponent>();
	context.add_tags<StaticObjectTag>();
	_world->get_entity_manager()->create_archetype(context);
}

void EngineObjectsCreator::setup_skeletal_model_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, ModelComponent, CastShadowComponent, VisibleComponent, OpaquePBRMaterialComponent>();
	context.add_tags<MovableObjectTag>();
	_world->get_entity_manager()->create_archetype(context);
}

void EngineObjectsCreator::setup_point_light_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, IntensityComponent, ColorComponent, AttenuationRadiusComponent,
		LightTemperatureComponent, CastShadowComponent, VisibleComponent, AffectWorldComponent, ExtentComponent>();
	context.add_tags<StaticObjectTag, PointLightTag>();
	_world->get_entity_manager()->create_archetype(context);
}

void EngineObjectsCreator::setup_directional_light_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, IntensityComponent, ColorComponent,
		LightTemperatureComponent, CastShadowComponent, VisibleComponent, AffectWorldComponent, ExtentComponent>();
	context.add_tags<StaticObjectTag, DirectionalLightTag>();
	_world->get_entity_manager()->create_archetype(context);
}

void EngineObjectsCreator::setup_spot_light_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, IntensityComponent, ColorComponent, AttenuationRadiusComponent,
		LightTemperatureComponent, CastShadowComponent, VisibleComponent, AffectWorldComponent,
		OuterConeAngleComponent, InnerConeAngleComponent, ExtentComponent>();
	context.add_tags<StaticObjectTag, SpotLightTag>();
	_world->get_entity_manager()->create_archetype(context);
}

void EngineObjectsCreator::setup_camera_archetype()
{
	ecs::ArchetypeCreationContext context;
	context.set_entity_count(16);
	context.add_components<CameraComponent, TransformComponent>();
	_world->get_entity_manager()->create_archetype(context);
}

void EngineObjectsCreator::setup_basic_model_components(
	ecs::EntityCreationContext& entityCreationContext,
	EditorObjectCreationContext& objectCreationContext)
{
	entityCreationContext.add_component<TransformComponent>(objectCreationContext.location, objectCreationContext.rotation, objectCreationContext.scale);
	entityCreationContext.add_component<ModelComponent>(objectCreationContext.uuid);
	entityCreationContext.add_component<CastShadowComponent>(true);
	entityCreationContext.add_component<VisibleComponent>(true);
	entityCreationContext.add_component<OpaquePBRMaterialComponent>(objectCreationContext.materialUUID);
}

void EngineObjectsCreator::setup_basic_light_components(
	ecs::EntityCreationContext& entityCreationContext,
	EditorObjectCreationContext& objectCreationContext)
{
	entityCreationContext.add_component<TransformComponent>(objectCreationContext.location, objectCreationContext.rotation, objectCreationContext.scale);
	entityCreationContext.add_component<ColorComponent>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	entityCreationContext.add_component<LightTemperatureComponent>(false, 6500.0f);
	entityCreationContext.add_component<CastShadowComponent>(true);
	entityCreationContext.add_component<VisibleComponent>(true);
	entityCreationContext.add_component<AffectWorldComponent>(true);
	entityCreationContext.add_component<ExtentComponent>(2048u, 2048u);
}

