#pragma once

#include "resource_manager/resource_formats.h"

#include <vector>

namespace ad_astris::ecore
{
	class GameObject;
	
	class Level
	{
		// TODO
		public:
			Level();

		private:
			std::vector<GameObject> _gameObjects;
	};
}

namespace ad_astris::resource
{
	template<>
	struct ResourceMethods<ecore::Level>
	{
		static void* create(void* data);
		static void destroy(void* resource);
	};
}
