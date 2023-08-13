#pragma once

#include "engine_core/base_settings.h"
#include "renderer_subsettings.h"
#include "project_description_subsettings.h"

namespace ad_astris::ecore
{
	class ProjectSettings : public BaseSettings
	{
		public:
			void setup_default_settings(DefaultSettingsContext<ProjectSettings>& defaultSettingsContext);
			
		private:
			SETUP_BUILTIN_SUBSETTINGS(RendererSubsettings, ProjectDescriptionSubsettings)
			SETUP_SETTINGS_LOG_METHODS(ProjectSettings)
	};

	template<>
	struct DefaultSettingsContext<ProjectSettings>
	{
		io::URI defaultLevelPath;
		std::string projectName;
	};
}