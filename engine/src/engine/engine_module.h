#pragma once

#include "application/project_launcher_module.h"
#include "core/module.h"
#include "events/event_manager.h"
#include "application_core/window.h"
#include "file_system/file_system.h"

namespace ad_astris::engine
{
	struct EngineInitializationContext
	{
		io::FileSystem* fileSystem;
		acore::IWindow* mainWindow;
		ModuleManager* moduleManager;
		events::EventManager* eventManager;
		devtools::ProjectInfo* projectInfo;
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