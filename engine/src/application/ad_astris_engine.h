#pragma once

#include "events/event_manager.h"
#include "engine/engine_module.h"
#include "application_core/window.h"

namespace ad_astris::app
{
	class AdAstrisEngine
	{
		public:
			AdAstrisEngine(const io::URI& projectPath);

		private:
			std::unique_ptr<events::EventManager> _eventManager{ nullptr };
			std::unique_ptr<acore::IWindow> _window{ nullptr };
			std::unique_ptr<engine::IEngine> _engine{ nullptr };
	};
}