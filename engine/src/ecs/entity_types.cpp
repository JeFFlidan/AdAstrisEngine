#include "entity_types.h"
#include "core/global_objects.h"

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

bool Entity::has_component_internal(uint64_t componentID) const
{
	return ENTITY_MANAGER()->does_entity_have_component(*this, componentID);
}

bool Entity::has_tag_internal(uint64_t tagID) const
{
	return ENTITY_MANAGER()->does_entity_have_tag(*this, tagID);
}

void* Entity::get_component_by_id(uint64_t componentID) const
{
	return ENTITY_MANAGER()->get_entity_component_by_id(*this, componentID);
}

bool Entity::is_valid() const
{
	return ENTITY_MANAGER()->is_entity_valid(*this);
}

