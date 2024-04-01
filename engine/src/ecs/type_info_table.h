#pragma once

#include "attributes.h"
#include "core/reflection.h"
#include "profiler/logger.h"

#include <unordered_map>
#include <vector>
#include <mutex>

namespace ad_astris::ecs
{
	class TypeInfoTable
	{
		public:
			template<typename T>
			static constexpr uint64_t get_component_id()
			{
				if constexpr (Reflector::has_attribute<T, EcsComponent>())
				{
					return Reflector::get_attribute<T, EcsComponent>().get_id();
				}
				else
				{
					LOG_ERROR("TypeInfoTable::get_component_id(): {} does not have EcsComponent attribute", get_type_name<T>())
					return -1;
				}
			}
		
			template<typename T>
			static constexpr uint64_t get_tag_id()
			{
				if constexpr (Reflector::has_attribute<T, EcsTag>())
				{
					return Reflector::get_attribute<T, EcsTag>().get_id();
				}
				else
				{
					LOG_ERROR("TypeInfoTable::get_tag_id(): {} does not have EcsTag attribute", get_type_name<T>())
					return -1;
				}
			}
	
			template<typename T>
			uint32_t get_system_id()
			{
				std::string typeName = get_type_name<T>();
				auto it = _systemIDByName.find(typeName);
				if (it == _systemIDByName.end())
					return -1;
				
				return it->second;
			}
				
			uint32_t get_system_id(std::string& typeName)
			{
				auto it = _systemIDByName.find(typeName);
				if (it == _systemIDByName.end())
					return -1;
				
				return it->second;
			}

			std::string get_system_name(uint32_t typeID)
			{
				if (typeID >= _systemNames.size())
				{
					LOG_ERROR("TYPE_INFO_TABLE->get_system_name(): Type ID > than names count")
					return "UndefinedTag";
				}
				return _systemNames[typeID];
			}
				
			template<typename T>
			void add_system()
			{
				std::string typeName = get_type_name<T>();
				if (_systemIDByName.find(typeName) != _systemIDByName.end())
					return;
						
				_systemIDByName[typeName] = _systemIDGenerator++;
			}

		private:
			std::unordered_map<std::string, uint32_t> _systemIDByName;
			std::vector<std::string> _systemNames;   // element index = type id
			uint32_t _systemIDGenerator{ 0 };
	};

	inline TypeInfoTable* TYPE_INFO_TABLE{ nullptr };

	inline void create_type_info_table() { TYPE_INFO_TABLE = new TypeInfoTable(); }
	inline void destroy_type_info_table() { delete TYPE_INFO_TABLE; }
	inline TypeInfoTable* get_active_type_info_table() { return TYPE_INFO_TABLE; }
	inline void set_type_info_table(TypeInfoTable* table) { TYPE_INFO_TABLE = table; }
}
