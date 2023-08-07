#pragma once

#include "core/module.h"
#include <string>

namespace ad_astris::devtools
{
	class IProjectLauncher
	{
		public:
			virtual ~IProjectLauncher() { }
			virtual void init(io::FileSystem* fileSystem) = 0;
			virtual void draw_window() = 0;
			virtual std::string get_project_path() = 0;
	};
	
	class IProjectLauncherModule : IModule
	{
		public:
			virtual IProjectLauncher* get_project_launcher() = 0;
	};
}