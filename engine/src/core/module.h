#pragma once

namespace ad_astris
{
	class IModule
	{
		public:
			virtual ~IModule() { }

			virtual void startup_module() { }

			virtual void shutdown_module() { }
	};
}