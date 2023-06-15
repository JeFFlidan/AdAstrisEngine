#include "type_id_tables.h"

using namespace ad_astris::ecs;

ComponentTypeIDTable* ComponentTypeIDTable::_instance{ nullptr };
std::mutex ComponentTypeIDTable::_mutex;
uint32_t ComponentTypeIDTable::_idGenerator{ 0 };
std::unordered_map<std::string, ComponentTypeIDTable::ComponentInfo> ComponentTypeIDTable::_table;

std::unordered_map<std::string, uint32_t> TagTypeIdTable::_tableNameToId;
std::vector<std::string> TagTypeIdTable::_names;
uint32_t TagTypeIdTable::_idGenerator{ 0 };
