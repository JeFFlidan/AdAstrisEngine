#pragma once

#include "core/global_objects.h"
#include "application/project_launcher_module.h"
#include "application_core/window.h"
#include "rhi/ui_window_backend.h"
#include "ui_core/ecs_ui_manager.h"

namespace ad_astris::engine
{
	struct EngineInitializationContext
	{
		GlobalObjectContext* globalObjectContext{ nullptr };
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
	
	class IEngineModule : public IModule
	{
		public:
			virtual IEngine* get_engine() = 0;
	};
}