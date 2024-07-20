#pragma once

#include "application_core/window.h"
#include "application/project_launcher_module.h"

namespace ad_astris::engine
{
	struct EngineInitializationContext
	{
		acore::IWindow* mainWindow{ nullptr };
		devtools::ProjectInfo* projectInfo{ nullptr };
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