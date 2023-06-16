#include "world.h"

using namespace ad_astris;

void ecore::World::add_level(Level* level)
{
	_levels.insert(level);
	if (!_currentLevel)
		_currentLevel = level;
}
