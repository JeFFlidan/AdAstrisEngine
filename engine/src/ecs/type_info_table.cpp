#include "type_info_table.h"
#include "system.h"

using namespace ad_astris::ecs;

uint32_t TypeInfoTable::_componentIDGenerator{ 0 };
std::unordered_map<std::string, TypeInfoTable::ComponentInfo> TypeInfoTable::_componentInfoByName;
std::unordered_map<std::string, uint32_t> TypeInfoTable::_tagIDByName;
std::vector<std::string> TypeInfoTable::_tagNames;
uint32_t TypeInfoTable::_tagIDGenerator{ 0 };
std::unordered_map<std::string, uint32_t> TypeInfoTable::_systemIDByName;
uint32_t TypeInfoTable::_systemIDGenerator{ 0 };
std::vector<std::string> TypeInfoTable::_systemNames;
