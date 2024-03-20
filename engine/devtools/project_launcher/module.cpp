#include "project_launcher.h"
#include "core/module_manager.h"
#include "core/global_objects.h"

namespace ad_astris::devtools::pl_impl
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

	DECL_MODULE_FUNCS(ProjectLauncher, ProjectLauncherModule)
}