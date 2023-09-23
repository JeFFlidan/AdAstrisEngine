#pragma once

#include "rhi/engine_rhi.h"
#include "multithreading/task_composer.h"
#include "events/event_manager.h"
#include "resource_manager/resource_manager.h"
#include "engine/render_core_module.h"

namespace ad_astris::renderer::impl
{
	struct SceneManagerInitializationContext
	{
		rhi::IEngineRHI* rhi;
		events::EventManager* eventManager;
		tasks::TaskComposer* taskComposer;
		resource::ResourceManager* resourceManager;
		rcore::IRendererResourceManager* rendererResourceManager;
	};
	
	struct SceneSubmanagerInitializationContext : SceneManagerInitializationContext
	{
		
	};
	
	class SceneSubmanagerBase
	{
		public:
			SceneSubmanagerBase(SceneSubmanagerInitializationContext& initContext)
				: _rhi(initContext.rhi), _taskComposer(initContext.taskComposer), _eventManager(initContext.eventManager),
					_resourceManager(initContext.resourceManager), _rendererResourceManager(initContext.rendererResourceManager) { }
			virtual ~SceneSubmanagerBase() { }
		
			virtual void update(rhi::CommandBuffer& cmdBuffer) = 0;
			virtual void cleanup_after_update() = 0;
			virtual bool need_update() = 0;

		protected:
			rhi::IEngineRHI* _rhi{ nullptr };
			tasks::TaskComposer* _taskComposer{ nullptr };
			events::EventManager* _eventManager{ nullptr };
			resource::ResourceManager* _resourceManager{ nullptr };
			rcore::IRendererResourceManager* _rendererResourceManager{ nullptr };
		
			virtual void subscribe_to_events() = 0;
	};

}