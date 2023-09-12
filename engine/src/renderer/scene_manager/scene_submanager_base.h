﻿#pragma once

#include "rhi/engine_rhi.h"
#include "multithreading/task_composer.h"
#include "events/event_manager.h"
#include "resource_manager/resource_manager.h"

namespace ad_astris::renderer::impl
{
	struct SceneManagerInitializationContext
	{
		rhi::IEngineRHI* rhi;
		events::EventManager* eventManager;
		tasks::TaskComposer* taskComposer;
		resource::ResourceManager* resourceManager;
	};
	
	struct SceneSubmanagerInitializationContext : SceneManagerInitializationContext
	{
		
	};
	
	class SceneSubmanagerBase
	{
		public:
			SceneSubmanagerBase(SceneSubmanagerInitializationContext& initContext)
				: _rhi(initContext.rhi), _taskComposer(initContext.taskComposer), _eventManager(initContext.eventManager),
					_resourceManager(initContext.resourceManager) { }
			virtual ~SceneSubmanagerBase() { }
		
			virtual void update(rhi::CommandBuffer& cmdBuffer) = 0;
			virtual void cleanup_staging_buffers() = 0;
			virtual bool need_allocation() = 0;

		protected:
			rhi::IEngineRHI* _rhi{ nullptr };
			tasks::TaskComposer* _taskComposer{ nullptr };
			events::EventManager* _eventManager{ nullptr };
			resource::ResourceManager* _resourceManager{ nullptr };
		
			virtual void subscribe_to_events() = 0;
	};

}