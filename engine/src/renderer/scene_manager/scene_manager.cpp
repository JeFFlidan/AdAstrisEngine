#include "scene_manager.h"
#include "engine_core/basic_events.h"
#include "core/global_objects.h"

using namespace ad_astris::renderer::impl;

SceneManager::SceneManager()
{
	subscribe_to_events();

	_materialSubmanager = std::make_unique<MaterialSubmanager>();
	_modelSubmanager = std::make_unique<ModelSubmanager>(_materialSubmanager.get());
	_entitySubmanager = std::make_unique<EntitySubmanager>();
}

SceneManager::~SceneManager()
{
	
}

void SceneManager::setup_global_buffers()
{
	rhi::CommandBuffer transferCmdBuffer;
	RHI()->begin_command_buffer(&transferCmdBuffer, rhi::QueueType::TRANSFER);

	// ORDER IS IMPORTANT!!!
	_materialSubmanager->update(transferCmdBuffer);
	_modelSubmanager->update(transferCmdBuffer);
	_entitySubmanager->update(transferCmdBuffer);

	_materialSubmanager->cleanup_after_update();
	_modelSubmanager->cleanup_after_update();
	_entitySubmanager->cleanup_after_update();
	
	RHI()->submit(rhi::QueueType::TRANSFER, true);
	RENDERER_RESOURCE_MANAGER()->cleanup_staging_buffers();
}

void SceneManager::subscribe_to_events()
{
	events::EventDelegate<ecore::EntityCreatedEvent> delegate1 = [&](ecore::EntityCreatedEvent& event)
	{
		ecs::EntityManager* entityManager = event.get_entity_manager();
		ecs::Entity entity = event.get_entity();

		if (entityManager->does_entity_have_component<ecore::ModelComponent>(entity))
		{
			auto modelComponent = entityManager->get_component_const<ecore::ModelComponent>(entity);
			if (entityManager->does_entity_have_tag<ecore::StaticObjectTag>(entity))
			{
				auto handle = RESOURCE_MANAGER()->get_resource<ecore::StaticModel>(modelComponent->modelUUID);
				_modelSubmanager->add_static_model(handle, entity);
			}

			if (entityManager->does_entity_have_component<ecore::OpaquePBRMaterialComponent>(entity))
			{
				auto materialComponent = entityManager->get_component_const<ecore::OpaquePBRMaterialComponent>(entity);
				_materialSubmanager->add_cpu_opaque_material_uuid(materialComponent->materialUUID);
			}
		}

		if (entityManager->does_entity_have_tag<ecore::SpotLightTag>(entity))
			_entitySubmanager->add_light_entity(entity);
		else if (entityManager->does_entity_have_tag<ecore::PointLightTag>(entity))
			_entitySubmanager->add_light_entity(entity);
		else if (entityManager->does_entity_have_tag<ecore::DirectionalLightTag>(entity))
			_entitySubmanager->add_light_entity(entity);
		// TODO lights and textures
	};
	EVENT_MANAGER()->subscribe(delegate1);
}
