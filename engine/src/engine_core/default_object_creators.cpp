#include "default_object_creators.h"
#include "core/global_objects.h"
#include "engine_core/basic_components.h"

using namespace ad_astris;
using namespace ecore;

void setup_basic_model_components(
	ecs::EntityCreationContext& entityCreationContext,
	EditorObjectCreationContext& objectCreationContext)
{
	TransformComponent transformComponent;
	transformComponent.location = objectCreationContext.location;
	transformComponent.rotationEuler = objectCreationContext.rotationEuler;
	transformComponent.scale = objectCreationContext.scale;
	entityCreationContext.add_component<TransformComponent>(transformComponent);
	entityCreationContext.add_component<ModelComponent>(objectCreationContext.uuid);
	entityCreationContext.add_component<CastShadowComponent>(true);
	entityCreationContext.add_component<VisibleComponent>(true);
	entityCreationContext.add_component<OpaquePBRMaterialComponent>(objectCreationContext.materialUUID);
}

void setup_basic_light_components(
	ecs::EntityCreationContext& entityCreationContext,
	EditorObjectCreationContext& objectCreationContext)
{
	TransformComponent transformComponent;
	transformComponent.location = objectCreationContext.location;
	transformComponent.rotationEuler = objectCreationContext.rotationEuler;
	transformComponent.scale = objectCreationContext.scale;
	entityCreationContext.add_component<TransformComponent>(transformComponent);
	entityCreationContext.add_component<ColorComponent>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	entityCreationContext.add_component<LightTemperatureComponent>(false, 6500.0f);
	entityCreationContext.add_component<CastShadowComponent>(true);
	entityCreationContext.add_component<VisibleComponent>(true);
	entityCreationContext.add_component<AffectWorldComponent>(true);
	entityCreationContext.add_component<ExtentComponent>(2048u, 2048u);
}

void StaticModelCreator::init()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, ModelComponent, CastShadowComponent, VisibleComponent, OpaquePBRMaterialComponent>();
	context.add_tags<StaticObjectTag>();
	WORLD()->get_entity_manager()->create_archetype(context);
}

ecs::Entity StaticModelCreator::create(EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_model_components(entityCreationContext, objectCreationContext);
	entityCreationContext.add_tag<StaticObjectTag>();
	
	return WORLD()->create_entity(entityCreationContext);
}

void SkeletalModelCreator::init()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, ModelComponent, CastShadowComponent, VisibleComponent, OpaquePBRMaterialComponent>();
	context.add_tags<MovableObjectTag>();
	WORLD()->get_entity_manager()->create_archetype(context);
}

ecs::Entity SkeletalModelCreator::create(EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_model_components(entityCreationContext, objectCreationContext);
	entityCreationContext.add_tag<MovableObjectTag>();

	return WORLD()->create_entity(entityCreationContext);
}

void StaticPointLightCreator::init()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, IntensityComponent, ColorComponent, AttenuationRadiusComponent,
		LightTemperatureComponent, CastShadowComponent, VisibleComponent, AffectWorldComponent, ExtentComponent>();
	context.add_tags<StaticObjectTag, PointLightTag>();
	WORLD()->get_entity_manager()->create_archetype(context);
}

ecs::Entity StaticPointLightCreator::create(EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_light_components(entityCreationContext, objectCreationContext);
	entityCreationContext.add_component<IntensityComponent>(8.0f);
	entityCreationContext.add_component<AttenuationRadiusComponent>(1000.0f);
	entityCreationContext.add_tag<StaticObjectTag>();
	entityCreationContext.add_tag<PointLightTag>();

	return WORLD()->create_entity(entityCreationContext);
}

void StaticSpotLightCreator::init()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, IntensityComponent, ColorComponent, AttenuationRadiusComponent,
		LightTemperatureComponent, CastShadowComponent, VisibleComponent, AffectWorldComponent,
		OuterConeAngleComponent, InnerConeAngleComponent, ExtentComponent>();
	context.add_tags<StaticObjectTag, SpotLightTag>();
	WORLD()->get_entity_manager()->create_archetype(context);
}

ecs::Entity StaticSpotLightCreator::create(EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_light_components(entityCreationContext, objectCreationContext);
	entityCreationContext.add_component<IntensityComponent>(8.0f);
	entityCreationContext.add_component<AttenuationRadiusComponent>(1000.0f);
	entityCreationContext.add_component<OuterConeAngleComponent>(45.0f);
	entityCreationContext.add_component<InnerConeAngleComponent>(0.0f);
	entityCreationContext.add_tag<StaticObjectTag>();
	entityCreationContext.add_tag<SpotLightTag>();

	return WORLD()->create_entity(entityCreationContext);
}

void StaticDirectionalLightCreator::init()
{
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, IntensityComponent, ColorComponent,
		LightTemperatureComponent, CastShadowComponent, VisibleComponent, AffectWorldComponent, ExtentComponent>();
	context.add_tags<StaticObjectTag, DirectionalLightTag>();
	WORLD()->get_entity_manager()->create_archetype(context);
}

ecs::Entity StaticDirectionalLightCreator::create(EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	setup_basic_light_components(entityCreationContext, objectCreationContext);
	entityCreationContext.add_component<IntensityComponent>(1.0f);
	entityCreationContext.add_tag<StaticObjectTag>();
	entityCreationContext.add_tag<DirectionalLightTag>();

	return WORLD()->create_entity(entityCreationContext);
}

void CameraCreator::init()
{
	ecs::ArchetypeCreationContext context;
	context.set_entity_count(16);
	context.add_components<CameraComponent, TransformComponent>();
	WORLD()->get_entity_manager()->create_archetype(context);
}

ecs::Entity CameraCreator::create(EditorObjectCreationContext& objectCreationContext)
{
	ecs::EntityCreationContext entityCreationContext;
	CameraComponent cameraComponent{};
	cameraComponent.movementSpeed = 60.0f; 
	cameraComponent.mouseSensitivity = 0.12f;
	entityCreationContext.add_component(cameraComponent);

	TransformComponent transformComponent;
	transformComponent.location = objectCreationContext.location;
	transformComponent.rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	transformComponent.scale = objectCreationContext.scale;
	entityCreationContext.add_component(transformComponent);

	return WORLD()->create_entity(entityCreationContext);
}
