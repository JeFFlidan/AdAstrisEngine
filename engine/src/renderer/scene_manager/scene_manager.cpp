#include "scene_manager.h"
#include "core/reflection.h"

using namespace ad_astris::renderer::impl;

SceneManager::SceneManager(SceneManagerInitializationContext& initContext)
	: _rhi(initContext.rhi), _eventManager(initContext.eventManager), _taskComposer(initContext.taskComposer),
	_rendererResourceManager(initContext.rendererResourceManager)
{
	SceneSubmanagerInitializationContext submanagerInitContext;
	submanagerInitContext.rhi = initContext.rhi;
	submanagerInitContext.eventManager = initContext.eventManager;
	submanagerInitContext.taskComposer = initContext.taskComposer;
	submanagerInitContext.rendererResourceManager = initContext.rendererResourceManager;
	submanagerInitContext.resourceManager = initContext.resourceManager;

	add_submanager<ModelSubmanager>(submanagerInitContext);
	add_submanager<LightSubmanager>(submanagerInitContext);
}

SceneManager::~SceneManager()
{
	
}

void SceneManager::setup_global_buffers()
{
	for (auto& pair : _submanagerByItsName)
	{
		if (pair.second->need_update())
			_submanagersToUpdate.push_back(pair.second.get());
	}

	rhi::CommandBuffer transferCmdBuffer;
	_rhi->begin_command_buffer(&transferCmdBuffer, rhi::QueueType::TRANSFER);

	for (auto& submanager : _submanagersToUpdate)
	{
		submanager->update(transferCmdBuffer);
		submanager->cleanup_after_update();
	}
	
	_rhi->submit(rhi::QueueType::TRANSFER, true);
	_rendererResourceManager->cleanup_staging_buffers();
	_submanagersToUpdate.clear();
}
