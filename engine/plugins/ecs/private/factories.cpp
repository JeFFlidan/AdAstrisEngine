#include "factories.h"
#include "profiler/logger.h"

using namespace ad_astris::ecs;
using namespace factories;

void BaseFactory::build(
	EntityCreationContext& creationContext,
	uint32_t componentTypeId,
	std::string& componentName,
	nlohmann::json& jsonWithComponents)
{
	serializers::BaseSerializer* serializer = serializers::get_table()->get_serializer(componentTypeId);
	build_object(creationContext, componentName, jsonWithComponents, serializer);
}

FactoriesTable* FactoriesTable::_instance{ nullptr };
std::mutex FactoriesTable::_mutex;

FactoriesTable* FactoriesTable::get_instance()
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (!_instance)
		_instance = new FactoriesTable();
	return _instance;
}

BaseFactory* FactoriesTable::get_factory(uint32_t componentTypeId)
{
	auto factoryIterator = _typeIdToFactory.find(componentTypeId);
	if (factoryIterator == _typeIdToFactory.end())
	{
		LOG_ERROR("FactoriesTable::get_factory(): There is no factory by component type id {}", componentTypeId)
		return nullptr;
	}

	return factoryIterator->second;
}
