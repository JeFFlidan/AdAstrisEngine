#pragma once

#include "api.h"
#include "application_core/glfw_window/glfw_window.h"
#include "application/project_launcher_module.h"
#include <memory>
#include <string>

namespace ad_astris::devtools::impl
{
	class PROJECT_LAUNCHER_API ProjectLauncher final : public IProjectLauncher
	{
		public:
			ProjectLauncher();
			virtual void draw_window() override;

			virtual std::string get_project_path() override
			{
				return _projectPath;
			}

		private:
			std::unique_ptr<acore::GlfwWindow> _glfwWindow{ nullptr };
			std::string _projectPath;

			void create_window();
	};
}