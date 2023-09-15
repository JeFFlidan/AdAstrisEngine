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

BaseSerializer* SerializersTable::get_serializer(uint32_t componentTypeID)
{
	auto serializerIterator = _serializerByTypeID.find(componentTypeID);
	if (serializerIterator == _serializerByTypeID.end())
	{
		LOG_ERROR("FactoriesTable::get_factory(): There is no factory by component type id {}", componentTypeID)
		return nullptr;
	}

	return serializerIterator->second;
}
