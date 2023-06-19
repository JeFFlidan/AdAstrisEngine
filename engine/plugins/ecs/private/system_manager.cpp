#include "system_manager.h"

#include "core/DAG.h"
#include "profiler/logger.h"

#include <iostream>

using namespace ad_astris::ecs;

std::vector<EntityManager*> SystemManager::_entityManagers;
std::unordered_map<uint32_t, System*> SystemManager::_idToSystem;
std::vector<uint32_t> SystemManager::_executionOrder; 

void SystemManager::init()
{
	
}

void SystemManager::cleanup()
{
	for (auto& data : _idToSystem)
	{
		delete data.second;
	}
}

void SystemManager::execute()
{
	std::vector<EntityManager*> managersForUpdate;
	for (auto& manager : _entityManagers)
	{
		if (!manager->_lastCreatedArchetypes.empty())
			managersForUpdate.push_back(manager);
	}
	
	for (auto& id : _executionOrder)
	{
		System* system = _idToSystem[id];
		update_queries(managersForUpdate, system);
		system->execute();
	}

	for (auto& manager : managersForUpdate)
	{
		manager->_lastCreatedArchetypes.clear();
	}
}

void SystemManager::add_entity_manager(EntityManager* entityManager)
{
	_entityManagers.push_back(entityManager);
}

void SystemManager::generate_execution_order()
{
	DAG* executionDAG = new DAG(_idToSystem.size());
	_executionOrder.clear();
	
	for (auto& systemInfo : _idToSystem)
	{
		uint32_t systemID = systemInfo.first;
		System* system = systemInfo.second;
		SystemExecutionOrder& executionOrder = system->_executionOrder;

		if (executionOrder._executeAfter.empty() && executionOrder._executeBefore.empty())
		{
			continue;
		}

		for (auto& executeBeforeName : executionOrder._executeBefore)
		{
			uint32_t executeBeforeID = SystemTypeIDTable::get_type_id(executeBeforeName);
			executionDAG->add_edge(systemID, executeBeforeID);
		}

		for (auto& executeAfterName : executionOrder._executeAfter)
		{
			uint32_t executeAfterID = SystemTypeIDTable::get_type_id(executeAfterName);
			executionDAG->add_edge(executeAfterID, systemID);
		}
	}
	
	executionDAG->topological_sort(_executionOrder);

	// FOR DEBUG
	// for (auto& id : _executionOrder)
	// {
	// 	std::cout << SystemTypeIDTable::get_system_name(id) << " ";
	// }

	delete executionDAG;
}

void SystemManager::update_queries(std::vector<EntityManager*>& managersForUpdate, System* system)
{
	EntityQuery& query = system->_entityQuery;
	uint32_t oldQueryArchetypesCount = query._archetypes.size();

	for (auto& manager : managersForUpdate)
	{
		for (auto& archetypeID : manager->_lastCreatedArchetypes)
		{
			Archetype& archetype = manager->_archetypes[archetypeID];
			if (archetype.check_requirements_match(query._requiredComponentIDs, query._requiredTagIDs))
			{
				query._archetypes.push_back(&archetype);
			}
		}
	}

	if (oldQueryArchetypesCount != query._archetypes.size())
	{
		for (int i = oldQueryArchetypesCount; i != query._archetypes.size(); ++i)
		{
			Archetype* archetype = query._archetypes[i];
			query._executionContexts.emplace_back(archetype);

			// I used this bad practise (in my opinion) because for unknown reason I can't pass
			// _componentIDToAccess unordered_map to the ExecutionContext constructor
			query._executionContexts.back()._componentIDToAccess = query._componentIDToAccess;
		}
	}
}
