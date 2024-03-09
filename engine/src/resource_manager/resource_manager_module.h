#pragma once

#include "core/module.h"

namespace ad_astris::resource::experimental
{
	class ResourceManager;
	class IResourceManagerModule : public IModule
	{
		public:
			virtual ResourceManager* get_resource_manager() = 0;
	};
}