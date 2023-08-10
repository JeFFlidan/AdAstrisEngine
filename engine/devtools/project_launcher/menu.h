#pragma once

#include "api.h"
#include "ui_core/widget_selection_manager.h"
#include <string>
#include <memory>

namespace ad_astris::devtools::pl_impl
{
	enum class MenuSection
	{
		RECENT_PROJECTS,
		CREATE_NEW_GAME
	};
	
	class PROJECT_LAUNCHER_API Menu
	{
		public:
			Menu() = default;
			Menu(std::string& rootPath);

			MenuSection draw_ui();

		private:
			std::unique_ptr<uicore::WidgetSelectionManager> _highlightManager;
			MenuSection _lastChosenMenuSection;
	};
}