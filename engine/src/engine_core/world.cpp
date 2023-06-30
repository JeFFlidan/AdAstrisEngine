#include "world.h"

using namespace ad_astris;
using namespace ecore;

void World::init()
{

}

void World::cleanup()
{

}

ecs::EntityManager* World::get_entity_manager()
{
	return &_entityManager;
}

void World::add_level(Level* level)
{
	level->_owningWorld = this;
	level->_entityManager = &_entityManager;
	_allLevels.insert(level);
	if (!_currentLevel)
		_currentLevel = level;
}

Level* World::get_current_level()
{
	return _currentLevel;
}

void World::serialize(io::IFile* file)
{
	
}

void World::deserialize(io::IFile* file, ObjectName* newName)
{
	
}
