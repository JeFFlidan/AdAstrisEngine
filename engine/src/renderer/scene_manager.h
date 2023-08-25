#pragma once

#include "rhi/resources.h"
#include "rhi/engine_rhi.h"
#include "events/event_handler.h"
#include "resource_manager/resource_events.h"

namespace ad_astris::renderer
{
	class SceneManager
	{
		public:
			SceneManager(rhi::IEngineRHI* rhi);
			~SceneManager();
		
		private:
			rhi::IEngineRHI* _rhi;

			events::EventHandler<resource::StaticModelLoadedEvent> _staticModelLoadedDelegate;
			events::EventHandler<resource::Texture2DLoadedEvent> _texture2DLoadedDelegate;

			void setup_event_delegates();
	};
}