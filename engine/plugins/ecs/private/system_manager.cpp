#include "system_manager.h"

using namespace ad_astris::ecs;

std::vector<EntityManager*> SystemManager::_entityManagers;

void SystemManager::execute()
{
	
}

void SystemManager::add_entity_manager(EntityManager* entityManager)
{
	_entityManagers.push_back(entityManager);
}
