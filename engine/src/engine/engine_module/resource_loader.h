#pragma once

#include "engine_core/material/materials.h"

namespace ad_astris::engine::impl
{
	class ResourceLoader
	{
		public:
			ResourceLoader();
			void load_new_resources();
		
		private:
			std::vector<std::pair<io::URI, io::URI>> _resourcePaths;
			std::vector<resource::FirstCreationContext<ecore::OpaquePBRMaterial>> _materialsToCreate;

			void subscribe_to_events();
	};
}