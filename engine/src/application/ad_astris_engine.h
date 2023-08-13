#pragma once

#include "events/event_manager.h"
#include "file_system/file_system.h"
#include "core/module_manager.h"
#include "engine/engine_module.h"
#include "application_core/window.h"

namespace ad_astris::app
{
	class AdAstrisEngine
	{
		public:
			bool init();
			void execute();
			void save_and_cleanup();

		private:
			std::unique_ptr<events::EventManager> _eventManager{ nullptr };
			std::unique_ptr<ModuleManager> _moduleManager{ nullptr };
			std::unique_ptr<io::FileSystem> _fileSystem{ nullptr };
			std::unique_ptr<acore::IWindow> _mainWindow{ nullptr };
			engine::IEngine* _engine{ nullptr };
	};
}