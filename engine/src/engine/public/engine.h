#pragma once

#include "rhi/ui_window_backend.h"
#include "application_core/window.h"
#include "application/project_launcher_module.h"

namespace ad_astris::engine
{
	struct EngineInitializationContext
	{
		acore::IWindow* mainWindow{ nullptr };
		devtools::ProjectInfo* projectInfo{ nullptr };
		rhi::UIWindowBackendCallbacks uiBackendCallbacks;	// Need this to setup ui in the application module
	};
	
	class IEngine
	{
		public:
			virtual ~IEngine() { }
			virtual void init(EngineInitializationContext& initializationContext) = 0;
			virtual void execute() = 0;
			virtual void save_and_cleanup(bool needToSave) = 0;
	};
}