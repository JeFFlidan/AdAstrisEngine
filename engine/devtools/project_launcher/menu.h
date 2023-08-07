#pragma once

#include "api.h"
#include "utils.h"
#include <string>

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
			TextureInfo _recentProjectsTextureInfo;
			TextureInfo _createNewGameTextureInfo;
			MenuSection _lastChosenMenuSection;
	};
}