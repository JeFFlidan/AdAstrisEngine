#include "type_id_tables.h"

using namespace ad_astris::ecs;

uint32_t ComponentTypeIDTable::_idGenerator{ 0 };
std::unordered_map<std::string, ComponentTypeIDTable::ComponentInfo> ComponentTypeIDTable::_table;

std::unordered_map<std::string, uint32_t> TagTypeIDTable::_tableNameToId;
std::vector<std::string> TagTypeIDTable::_names;
uint32_t TagTypeIDTable::_idGenerator{ 0 };

std::unordered_map<std::string, uint32_t> SystemTypeIDTable::_tableNameToId;
uint32_t SystemTypeIDTable::_idGenerator{ 0 };
std::vector<std::string> SystemTypeIDTable::_names;