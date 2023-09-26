#include "scene_manager.h"
#include "engine_core/basic_events.h"

using namespace ad_astris::renderer::impl;

SceneManager::SceneManager(SceneManagerInitializationContext& initContext)
	: _rhi(initContext.rhi), _eventManager(initContext.eventManager), _taskComposer(initContext.taskComposer),
	_rendererResourceManager(initContext.rendererResourceManager), _resourceManager(initContext.resourceManager)
{
	SceneSubmanagerInitializationContext submanagerInitContext;
	submanagerInitContext.rhi = initContext.rhi;
	submanagerInitContext.eventManager = initContext.eventManager;
	submanagerInitContext.taskComposer = initContext.taskComposer;
	submanagerInitContext.rendererResourceManager = initContext.rendererResourceManager;
	submanagerInitContext.resourceManager = initContext.resourceManager;
	submanagerInitContext.world = initContext.world;

	subscribe_to_events();
	
	add_submanager<ModelSubmanager>(submanagerInitContext);
	//add_submanager<LightSubmanager>(submanagerInitContext);
}

SceneManager::~SceneManager()
{
	
}

void SceneManager::setup_global_buffers()
{
	rhi::CommandBuffer transferCmdBuffer;
	_rhi->begin_command_buffer(&transferCmdBuffer, rhi::QueueType::TRANSFER);

	for (auto& pair : _submanagerByItsName)
	{
		pair.second->update(transferCmdBuffer);
		pair.second->cleanup_after_update();
	}
	
	_rhi->submit(rhi::QueueType::TRANSFER, true);
	_rendererResourceManager->cleanup_staging_buffers();
	_submanagersToUpdate.clear();
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
				auto handle = _resourceManager->get_resource<ecore::StaticModel>(modelComponent->modelUUID);
				get_model_submanager()->add_static_model(handle, entity);
			}
		}
		// TODO lights and textures
	};
	_eventManager->subscribe(delegate1);
}
