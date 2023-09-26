#include "world.h"

using namespace ad_astris;
using namespace ecore;

World::World(WorldCreationContext& creationContext) : _eventManager(creationContext.eventManager), _taskComposer(creationContext.taskComposer)
{
	_entityManager = std::make_unique<ecs::EntityManager>();
}

ecs::Entity World::create_entity(ecs::EntityCreationContext& creationContext)
{
	ecs::Entity entity = _entityManager->create_entity(creationContext);
	_currentLevel->add_entity(entity);
	return entity;
}

void World::add_entity(ecs::Entity& entity)
{
	_currentLevel->add_entity(entity);
}

void World::add_level(Level* level, bool isActive, bool isDefault)
{
	level->set_owning_world(this);
	_allLevels.insert(level);
	if (isActive)
	{
		_activeLevels.insert(level);
		_currentLevel = level;
	}
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

void World::serialize(io::File* file)
{
	
}

void World::deserialize(io::File* file, ObjectName* newName)
{
	
}
