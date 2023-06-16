#pragma once

#include "engine_core/world.h"
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
			ecore::World* _world;
			io::FileSystem* _fileSystem{ nullptr };
	};
}