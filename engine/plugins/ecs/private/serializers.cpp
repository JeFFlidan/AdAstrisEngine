#include "serializers.h"

using namespace ad_astris::ecs::serializers;

SerializersTable* SerializersTable::_instance{ nullptr };
std::mutex SerializersTable::_mutex;

SerializersTable* SerializersTable::get_instance()
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (!_instance)
		_instance = new SerializersTable();
	return _instance;
}

BaseSerializer* SerializersTable::get_serializer(uint32_t componentTypeId)
{
	auto serializerIterator = _typeIdToFactory.find(componentTypeId);
	if (serializerIterator == _typeIdToFactory.end())
	{
		LOG_ERROR("FactoriesTable::get_factory(): There is no factory by component type id {}", componentTypeId)
		return nullptr;
	}

	return serializerIterator->second;
}
