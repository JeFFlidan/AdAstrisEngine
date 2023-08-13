#include "project_description_subsettings.h"
#include "core/reflection.h"

using namespace ad_astris;
using namespace ecore;

void ProjectDescriptionSubsettings::serialize(Config& config)
{
	Section section(get_type_name<ProjectDescriptionSubsettings>());
	section.set_option("ProjectName", _projectName);
	section.set_option("ProjectVersion", _projectVersion);
	section.set_option("DefaultLevelPath", _defaultLevelPath);
	config.set_section(section);
}

void ProjectDescriptionSubsettings::deserialize(Section& section)
{
	_projectName = section.get_option_value<std::string>("ProjectName");
	_projectVersion = section.get_option_value<std::string>("ProjectVersion");
	_defaultLevelPath = section.get_option_value<std::string>("DefaultLevelPath");
}

void ProjectDescriptionSubsettings::setup_default_values()
{
	_projectVersion = "1.0.0.0";
}
