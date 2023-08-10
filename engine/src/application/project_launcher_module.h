﻿#pragma once

#include "core/module.h"
#include "file_system/file_system.h"
#include <string>

namespace ad_astris::devtools
{
	enum class NewProjectTemplate
	{
		OLD_PROJECT,
		BLANK
	};
	
	struct ProjectInfo
	{
		std::string projectPath;
		NewProjectTemplate newProjectTemplate{ NewProjectTemplate::OLD_PROJECT };
	};
	
	class IProjectLauncher
	{
		public:
			virtual ~IProjectLauncher() { }
			virtual void init(io::FileSystem* fileSystem) = 0;
			virtual void draw_window() = 0;
			virtual ProjectInfo get_project_info() = 0;
	};
	
	class IProjectLauncherModule : IModule
	{
		public:
			virtual IProjectLauncher* get_project_launcher() = 0;
	};
}