#pragma once

#include "engine.h"
#include "ui_core/ecs_ui_manager.h"

namespace ad_astris::engine
{
	class IEngineModule : public IModule
	{
		public:
			virtual IEngine* get_engine() = 0;
	};
}