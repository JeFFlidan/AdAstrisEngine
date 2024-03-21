#pragma once

#include "engine_core/project_settings/project_settings.h"
#include "application/project_launcher_module.h"

namespace ad_astris::engine::impl
{
	class Project
	{
		public:
			void load(const devtools::ProjectInfo& projectInfo);
			void postload(const devtools::ProjectInfo& projectInfo);
			void save();
		
			ecore::ProjectSettings* get_settings() const
			{
				return _projectSettings.get();
			}
			
		private:
			std::unique_ptr<ecore::ProjectSettings> _projectSettings{ nullptr };

			void load_existing_project();
			void create_new_blank_project();
			void create_default_material(const io::URI& engineRootPath, const io::URI& projectRootPath);
			UUID get_default_material_uuid();
	};
}