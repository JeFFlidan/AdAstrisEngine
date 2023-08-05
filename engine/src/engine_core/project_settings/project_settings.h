#pragma once

#include "engine_core/base_settings.h"
#include "renderer_subsettings.h"

namespace ad_astris::ecore
{
	class ProjectSettings : public BaseSettings
	{
		private:
			SETUP_BUILTIN_SUBSETTINGS(RendererSubsettings)
			SETUP_SETTINGS_LOG_METHODS(ProjectSettings)
	};
}