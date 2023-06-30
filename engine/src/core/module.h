#pragma once

namespace ad_astris
{
	class ModuleManager;
	// TODO: Have to think how to manager resources from modules (deleting)
	class IModule
	{
		public:
			virtual ~IModule() { }

			virtual void startup_module(ModuleManager* moduleManager) { }

			virtual void shutdown_module(ModuleManager* moduleManager) { }
	};
}