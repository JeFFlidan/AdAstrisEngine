#include "entity_types.h"

using namespace ad_astris::ecs;
using namespace ad_astris;

template <typename T>
Component<T>::Component(void* memory) : _memory(memory)
{
	_structureSize = sizeof(T);
}

template <typename T>
uint32_t Component<T>::get_type_id()
{
	return _typeId;
}

template <typename T>
uint32_t Component<T>::get_structure_size()
{
	return _structureSize;
}

template <typename T>
void* Component<T>::get_raw_memory()
{
	return _memory;
}

template <typename T>
T* Component<T>::get_memory()
{
	return static_cast<T*>(_memory);
}

Entity::Entity()
{
	_uuid = UUID();
}

Entity::Entity(UUID uuid) : _uuid(uuid)
{
	
}

UUID Entity::get_uuid()
{
	return _uuid;
}

bool Entity::operator!=(const Entity& other)
{
	return _uuid != other._uuid;
}

bool Entity::operator==(const Entity& other)
{
	return _uuid = other._uuid;
}

Entity::operator uint64_t() const
{
	return _uuid;
}
