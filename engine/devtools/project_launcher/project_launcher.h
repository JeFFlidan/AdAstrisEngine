#pragma once

#include "api.h"
#include "ui_manager.h"
#include "core/config_base.h"
#include "application_core/glfw_window/glfw_window.h"
#include "application/project_launcher_module.h"
#include "file_system/file_system.h"
#include <memory>

namespace ad_astris::devtools::pl_impl
{
	class PROJECT_LAUNCHER_API ProjectLauncher final : public IProjectLauncher
	{
		public:
			virtual void init(io::FileSystem* fileSystem) override;
			virtual void draw_window() override;

			virtual ProjectInfo get_project_info() override
			{
				return _projectInfo;		
			}
		
		private:
			std::unique_ptr<acore::GlfwWindow> _glfwWindow{ nullptr };
			std::unique_ptr<UIManager> _uiManager{ nullptr };
			std::unique_ptr<Config> _config{ nullptr };
			io::FileSystem* _fileSystem{ nullptr };
			ProjectInfo _projectInfo;

			void create_window();
	};
}