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
	return _entityManager.get();
}

void World::add_level(Level* level, bool isActive, bool isDefault)
{
	level->set_owning_world(this);
	_allLevels.insert(level);
	if (isActive)
		_activeLevels.insert(level);
	if (isDefault)
		_defaultLevel = level;
}

void World::set_current_level(Level* level)
{
	_currentLevel = level;
}

void World::build_entities_for_active_levels()
{
	for (auto level : _activeLevels)
		level->build_entities();
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
