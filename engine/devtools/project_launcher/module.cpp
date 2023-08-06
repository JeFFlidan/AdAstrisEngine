#include "project_launcher.h"
#include "core/module_manager.h"

namespace ad_astris::devtools::impl
{
	class ProjectLauncherModule : public IProjectLauncherModule
	{
		public:
			virtual void startup_module(ModuleManager* moduleManager) override;
			virtual IProjectLauncher* get_project_launcher() override;

		private:
			std::unique_ptr<IProjectLauncher> _projectLauncher;
	};

	void ProjectLauncherModule::startup_module(ModuleManager* moduleManager)
	{
		_projectLauncher = std::make_unique<ProjectLauncher>();
	}

	IProjectLauncher* ProjectLauncherModule::get_project_launcher()
	{
		return _projectLauncher.get();
	}

	extern "C" PROJECT_LAUNCHER_API IProjectLauncherModule* register_module()
	{
		return new ProjectLauncherModule();
	}
}