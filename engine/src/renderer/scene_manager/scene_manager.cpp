#include "scene_manager.h"
#include "core/reflection.h"

using namespace ad_astris::renderer::impl;

SceneManager::SceneManager(SceneManagerInitializationContext& initContext)
	: _rhi(initContext.rhi), _eventManager(initContext.eventManager), _taskComposer(initContext.taskComposer)
{
	subscribe_to_events();
	
	SceneSubmanagerInitializationContext submanagerInitContext;
	submanagerInitContext.rhi = initContext.rhi;
	submanagerInitContext.eventManager = initContext.eventManager;
	submanagerInitContext.taskComposer = initContext.taskComposer;

	add_submanager<ModelSubmanager>(submanagerInitContext);
	add_submanager<LightSubmanager>(submanagerInitContext);
}

SceneManager::~SceneManager()
{
	
}

void SceneManager::setup_global_buffers()
{
	if (_submanagersToUpdate.empty())
		return;

	rhi::CommandBuffer transferCmdBuffer;
	_rhi->begin_command_buffer(&transferCmdBuffer, rhi::QueueType::TRANSFER);

	for (auto& submanagerName : _submanagersToUpdate)
		_submanagerByItsName[submanagerName]->update(transferCmdBuffer);
	
	_rhi->submit(rhi::QueueType::TRANSFER);
	_rhi->wait_fences();
	for (auto& submanagerName : _submanagersToUpdate)
	{
		_submanagerByItsName[submanagerName]->cleanup_staging_buffers();
	}
	_submanagersToUpdate.clear();
}

void SceneManager::subscribe_to_events()
{
	events::EventDelegate<ModelSubmanagerUpdatedEvent> modelSubmanagerDelegate = [&](ModelSubmanagerUpdatedEvent& event)
	{
		_submanagersToUpdate.insert(event.get_submanager_name());
	};
	_eventManager->subscribe(modelSubmanagerDelegate);

	events::EventDelegate<LightSubmanagerUpdatedEvent> lightSubmanagerDelegate = [&](LightSubmanagerUpdatedEvent& event)
	{
		_submanagersToUpdate.insert(event.get_submanager_name());
	};
	_eventManager->subscribe(lightSubmanagerDelegate);
}
