#pragma once

#include "rhi/engine_rhi.h"
#include "multithreading/task_composer.h"
#include "events/event_manager.h"
#include "resource_manager/resource_manager.h"
#include "engine/render_core_module.h"
#include "engine_core/world.h"

namespace ad_astris::renderer::impl
{
	class SceneSubmanagerBase
	{
		public:
			virtual ~SceneSubmanagerBase() { }
		
			virtual void update(rhi::CommandBuffer& cmdBuffer) = 0;
			virtual void cleanup_after_update() = 0;
			virtual bool need_update() = 0;

		protected:
			virtual void subscribe_to_events() = 0;
	};

}