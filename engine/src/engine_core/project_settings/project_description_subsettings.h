#pragma once

#include "engine_core/subsettings.h"

namespace ad_astris::ecore
{
	class ProjectDescriptionSubsettings : public ISubsettings
	{
		public:
			SUBSETTINGS_TYPE_DECL(ProjectDescriptionSubsettings)

			virtual void serialize(Config& config) override;
			virtual void deserialize(Section& section) override;
			virtual void setup_default_values() override;

			std::string get_project_version()
			{
				return _projectVersion;
			}

			void set_project_version(const std::string& projectVersion)
			{
				_projectVersion = projectVersion;
			}

			std::string get_project_name()
			{
				return _projectName;
			}

			void set_project_name(const std::string& projectName)
			{
				_projectName = projectName;
			}

			std::string get_default_level_path()
			{
				return _defaultLevelPath;
			}

			void set_default_level_path(const io::URI& defaultLevelPath)
			{
				_defaultLevelPath = defaultLevelPath.c_str();
			}

		private:
			std::string _projectVersion;
			std::string _projectName;
			std::string _defaultLevelPath;
	};
}