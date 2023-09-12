#include "system_manager.h"
#include "core/DAG.h"
#include "profiler/logger.h"

#include <cassert>
#include <memory>

using namespace ad_astris::ecs;

bool SystemManager::_isInitialized = false;

SystemManager::SystemManager()
{
	assert(!_isInitialized);
	_isInitialized = true;
}

SystemManager::~SystemManager()
{
	_isInitialized = false;
}

void SystemManager::init()
{
	std::unordered_map<uint32_t, System*>& addedSystems = registration::SystemStorage::systems;

	for (auto& data : addedSystems)
	{
		data.second->configure_query();
	}
	
	_idToSystem.insert(addedSystems.begin(), addedSystems.end());
	
	registration::SystemStorage::systems.clear();
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
	auto executionDAG = std::make_unique<DAG>(_idToSystem.size());
	_executionOrder.clear();
	
	for (auto& systemInfo : _idToSystem)
	{
		uint32_t systemID = systemInfo.first;
		System* system = systemInfo.second;
		SystemExecutionOrder& systemExecutionOrder = system->_executionOrder;

		if (systemExecutionOrder._executeAfter.empty() && systemExecutionOrder._executeBefore.empty())
		{
			// TODO think what I must do with systems without execution order
			continue;
		}

		for (auto& executeBeforeName : systemExecutionOrder._executeBefore)
		{
			uint32_t executeBeforeID = TypeInfoTable::get_system_id(executeBeforeName);
			executionDAG->add_edge(systemID, executeBeforeID);
		}

		for (auto& executeAfterName : systemExecutionOrder._executeAfter)
		{
			uint32_t executeAfterID = TypeInfoTable::get_system_id(executeAfterName);
			executionDAG->add_edge(executeAfterID, systemID);
		}
	}
	
	executionDAG->topological_sort(_executionOrder);
	
	for (auto& id : _executionOrder)
	{
		LOG_INFO("SYSTEM: {}", TypeInfoTable::get_system_name(id))
	}
}

void SystemManager::update_queries(std::vector<EntityManager*>& managersForUpdate, System* system)
{
	EntityQuery& query = system->_entityQuery;

	for (auto& entityManager : managersForUpdate)
	{
		for (auto& archetypeID : entityManager->_lastCreatedArchetypes)
		{
			Archetype& archetype = entityManager->_archetypes[archetypeID];
			if (archetype.check_requirements_match(query._requiredComponentIDs, query._requiredTagIDs))
			{
				query._archetypes.push_back(&archetype);
				query._executionContexts.emplace_back(&archetype, query._componentIDToAccess);
			}
		}
	}

	// if (oldQueryArchetypesCount != query._archetypes.size())
	// {
	// 	for (int i = oldQueryArchetypesCount; i != query._archetypes.size(); ++i)
	// 	{
	// 		Archetype* archetype = query._archetypes[i];
	// 	}
	// }
}