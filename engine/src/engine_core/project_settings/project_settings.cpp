#include "project_settings.h"

using namespace ad_astris::ecore;

void ProjectSettings::setup_default_settings(DefaultSettingsContext<ProjectSettings>& defaultSettingsContext)
{
	RendererSubsettings rendererSubsettings;
	rendererSubsettings.setup_default_values();
	add_new_subsettings(rendererSubsettings);

	ProjectDescriptionSubsettings projectDescriptionSubsettings;
	projectDescriptionSubsettings.setup_default_values();
	projectDescriptionSubsettings.set_default_level_path(defaultSettingsContext.defaultLevelPath);
	projectDescriptionSubsettings.set_project_name(defaultSettingsContext.projectName);
	add_new_subsettings(projectDescriptionSubsettings);
}
