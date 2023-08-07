#pragma once

#include "api.h"
#include "core/config_base.h"
#include <string>

namespace ad_astris::devtools::pl_impl
{
	class PROJECT_LAUNCHER_API RecentProjectsBrowser
 	{
 		public:
 			RecentProjectsBrowser(std::string& rootPath, Config* config);
 
 			bool draw_ui();
 			std::string get_project_path()
 			{
 				return _chosenProjectPath;
 			}
 
 		private:
 			std::string _chosenProjectPath;
			Config* _config{ nullptr };
 	};
 }