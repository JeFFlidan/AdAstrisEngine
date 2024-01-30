#pragma once

#include "core/global_objects.h"
#include "engine/engine_module.h"
#include "application_core/window.h"
#include "editor_module.h"

namespace ad_astris::app
{
	class AdAstrisEngine
	{
		public:
			bool init();
			void execute();
			void save_and_cleanup();

		private:
			std::unique_ptr<GlobalObjectContext> _globalObjectContext{ nullptr };
			std::unique_ptr<acore::IWindow> _mainWindow{ nullptr };
			editor::IEditor* _editor{ nullptr };
			engine::IEngine* _engine{ nullptr };
			devtools::ProjectInfo _projectInfo;

			void init_global_objects();
			bool open_project();
			void init_main_window();
			void init_engine();
	};
}