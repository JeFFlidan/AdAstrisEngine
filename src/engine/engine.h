#pragma once

#include "resource_manager/resource_manager.h"
#include "file_system/file_system.h"
#include <memory>

namespace ad_astris
{
	class Engine
	{
		public:
			Engine();

		private:
			std::unique_ptr<resource::ResourceManager> _resourceManager;
			io::FileSystem* _fileSystem{ nullptr };
	};
}