#pragma once

#include "application/project_launcher_module.h"
#include "core/module.h"
#include "events/event_manager.h"
#include "application_core/window.h"
#include "file_system/file_system.h"
#include "rhi/ui_window_backend.h"
#include "ui_core/ecs_ui_manager.h"
#include <functional>

namespace ad_astris::engine
{
	struct EngineInitializationContext
	{
		io::FileSystem* fileSystem;
		acore::IWindow* mainWindow;
		ModuleManager* moduleManager;
		events::EventManager* eventManager;
		devtools::ProjectInfo* projectInfo;
		rhi::UIWindowBackendCallbacks uiBackendCallbacks;	// Need this to setup ui in the application module
	};
	
	class IEngine
	{
		public:
			virtual ~IEngine() { }
			virtual void init(EngineInitializationContext& initializationContext) = 0;
			virtual void execute() = 0;
			virtual void save_and_cleanup(bool needToSave) = 0;

			virtual uicore::ECSUiManager* get_ecs_ui_manager() = 0;
	};
	
	class IEngineModule : public IModule
	{
		public:
			virtual IEngine* get_engine() = 0;
	};
}