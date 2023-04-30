#pragma once

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
