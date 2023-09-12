#include "entity_types.h"

using namespace ad_astris::ecs;
using namespace ad_astris;

// ComponentTypeIDTable* get_type_id_table()
// {
// 	return ComponentTypeIDTable::get_instance();
// }

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
