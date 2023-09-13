#include "ecs/ecs.h"
#include "multithreading/task_composer.h"
#include "events/event_manager.h"
#include "engine_core/basic_components.h"
#include "core/timer.h"

using namespace ad_astris;

ecs::EntityManager* ENTITY_MANAGER = nullptr;
tasks::TaskComposer* TASK_COMPOSER = nullptr;

void setup_archetypes()
{
	Timer timer;
	using namespace ecore;
	ecs::ArchetypeCreationContext context;
	context.add_components<TransformComponent, LuminanceIntensityComponent, ColorComponent, AttenuationRadiusComponent,
		LightTemperatureComponent, CastShadowComponent, VisibleComponent, AffectWorldComponent, ExtentComponent>();
	context.add_tags<StaticObjectTag, PointLightTag>();
	ENTITY_MANAGER->create_archetype(context);
	LOG_INFO("Created archetype. {} ms", timer.elapsed_milliseconds())
}

void setup_entity_creation_context(ecs::EntityCreationContext& creationContext)
{
	creationContext.add_component<ecore::TransformComponent>(glm::vec3{1.0f}, glm::vec3{1.0f}, glm::vec3{1.0f});
	creationContext.add_component<ecore::LuminanceIntensityComponent>(200.0f);
	creationContext.add_component<ecore::AttenuationRadiusComponent>(45.0f);
	creationContext.add_component<ecore::LightTemperatureComponent>(true, 5500.0f);
	creationContext.add_component<ecore::CastShadowComponent>(true);
	creationContext.add_component<ecore::VisibleComponent>(true);
	creationContext.add_component<ecore::AffectWorldComponent>(true);
	creationContext.add_component<ecore::ExtentComponent>(2048, 2048);
	creationContext.add_tag<ecore::StaticObjectTag>();
	creationContext.add_tag<ecore::PointLightTag>();
}

void create_point_lights()
{
	Timer timer;
	for (uint32_t i = 0; i != 500; ++i)
	{
		ecs::EntityCreationContext creationContext;
		setup_entity_creation_context(creationContext);
		ENTITY_MANAGER->create_entity(creationContext);
	}
	
	LOG_INFO("Created 500 point lights in one thread. {} ms", timer.elapsed_milliseconds())

	tasks::TaskGroup& taskGroup = *TASK_COMPOSER->allocate_task_group();
	// for (uint32_t i = 0; i != 500; ++i)
	// {
	// 	TASK_COMPOSER->execute(taskGroup, [](tasks::TaskExecutionInfo)
	// 	{
	// 		ecs::EntityCreationContext creationContext;
	// 		setup_entity_creation_context(creationContext);
	// 		ENTITY_MANAGER->create_entity(creationContext);
	// 	});
	// }

	TASK_COMPOSER->dispatch(taskGroup, 500, 50, [&](tasks::TaskExecutionInfo execInfo)
	{
		ecs::EntityCreationContext creationContex;
		setup_entity_creation_context(creationContex);
		ENTITY_MANAGER->create_entity(creationContex);
	});
	
	TASK_COMPOSER->wait(taskGroup);
	LOG_INFO("Created 500 point lights in several threads. {} ms", timer.elapsed_milliseconds())
}

int main()
{
	ENTITY_MANAGER = new ecs::EntityManager();
	TASK_COMPOSER = new tasks::TaskComposer();

	create_point_lights();
	LOG_INFO("Archetypes count: {}", ENTITY_MANAGER->get_archetypes_count())
}
