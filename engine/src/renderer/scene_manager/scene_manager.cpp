#include "scene_manager.h"
#include "engine_core/basic_events.h"
#include "core/global_objects.h"

using namespace ad_astris::renderer::impl;

SceneManager::SceneManager()
{
	subscribe_to_events();

	_materialSubmanager = std::make_unique<MaterialSubmanager>();
	_cullingSubmanager = std::make_unique<CullingSubmanager>();
	_modelSubmanager = std::make_unique<ModelSubmanager>(_materialSubmanager.get(), _cullingSubmanager.get());
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
	_cullingSubmanager->update(transferCmdBuffer);

	_materialSubmanager->cleanup_after_update();
	_modelSubmanager->cleanup_after_update();
	_entitySubmanager->cleanup_after_update();
	_cullingSubmanager->cleanup_after_update();
	RHI()->submit(rhi::QueueType::TRANSFER, true);
	RENDERER_RESOURCE_MANAGER()->cleanup_staging_buffers();
}

void SceneManager::subscribe_to_events()
{
	events::EventDelegate<ecore::EntityCreatedEvent> delegate1 = [&](ecore::EntityCreatedEvent& event)
	{
		ecs::Entity entity = event.get_entity();

		if (entity.has_component<ecore::ModelComponent>())
		{
			_modelSubmanager->add_model(entity);
			_materialSubmanager->add_material(entity);
		}

		if (entity.has_tag<ecore::SpotLightTag>())
			_entitySubmanager->add_light_entity(entity);
		else if (entity.has_tag<ecore::PointLightTag>())
			_entitySubmanager->add_light_entity(entity);
		else if (entity.has_tag<ecore::DirectionalLightTag>())
			_entitySubmanager->add_light_entity(entity);
		// TODO lights and textures
	};
	EVENT_MANAGER()->subscribe(delegate1);
}
