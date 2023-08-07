#pragma once

#include "api.h"
#include "menu.h"
#include "recent_projects_browser.h"
#include "create_new_game_browser.h"
#include "core/config_base.h"
#include "ui_core/docking_window.h"

#include <application_core/glfw_window/glfw_window.h>
#include <string>

namespace ad_astris::devtools::pl_impl
{
	class PROJECT_LAUNCHER_API UIManager
	{
		public:
			UIManager(std::string& rootPath, Config* config, acore::GlfwWindow* glfwWindow);

			void cleanup();
			bool draw_ui();

		private:
			uicore::DockingWindow _dockingWindow;
			Menu _menu;
			RecentProjectsBrowser _recentProjectsBrowser;
			CreateNewGameBrowser _createNewGameBrowser;
			acore::GlfwWindow* _glfwWindow{ nullptr };
			std::string _configPath;
	};
}