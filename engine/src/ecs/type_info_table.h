#pragma once

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
			bool check_component_in_table()
			{
				std::string typeName = get_type_name<T>();
				if (_componentInfoByName.find(typeName) != _componentInfoByName.end())
					return true;
				return false;
			}
		
			template<typename T>
			uint32_t get_component_id()
			{
				std::string typeName = get_type_name<T>();
				auto it = _componentInfoByName.find(typeName);
				if (it == _componentInfoByName.end())
					return -1;
		
				return it->second.id;
			}
		
			uint32_t get_component_id(std::string& typeName)
			{
				auto it = _componentInfoByName.find(typeName);
				if (it == _componentInfoByName.end())
					return -1;
		
				return it->second.id;
			}
		
			template<typename T>
			uint32_t get_component_size()
			{
				std::string typeName = get_type_name<T>();
				auto it = _componentInfoByName.find(typeName);
				if (it == _componentInfoByName.end())
					return -1;
		
				return it->second.structureSize;
			}
		
			uint32_t get_component_size(std::string& typeName)
			{
				auto it = _componentInfoByName.find(typeName);
				if (it == _componentInfoByName.end())
					return -1;
		
				return it->second.structureSize;
			}
		
			template<typename T>
			void add_component_info()
			{
				std::string typeName = get_type_name<T>();
				if (_componentInfoByName.find(typeName) != _componentInfoByName.end())
					return;
		
				ComponentInfo info;
				info.id = ++_componentIDGenerator;
				info.structureSize = sizeof(T);
				_componentInfoByName[typeName] = info;
			}

			template<typename T>
			bool check_tag_in_table()
			{
				std::string typeName = get_type_name<T>();
				if (_tagIDByName.find(typeName) != _tagIDByName.end())
					return true;
				return false;
			}
			
			template<typename T>
			uint32_t get_tag_id()
			{
				std::string typeName = get_type_name<T>();
				auto it = _tagIDByName.find(typeName);
				if (it == _tagIDByName.end())
					return -1;
			
				return it->second;
			}
			
			uint32_t get_tag_id(std::string& typeName)
			{
				auto it = _tagIDByName.find(typeName);
				if (it == _tagIDByName.end())
					return -1;
			
				return it->second;
			}

			std::string get_tag_name(uint32_t typeID)
			{
				if (typeID >= _tagNames.size())
				{
					LOG_ERROR("TYPE_INFO_TABLE->get_tag_name(): Type ID > than names count")
					return "UndefinedTag";
				}
				return _tagNames[typeID];
			}

			template<typename T>
			void add_tag()
			{
				std::string typeName = get_type_name<T>();
				if (_tagIDByName.find(typeName) != _tagIDByName.end())
					return;
					
				_tagIDByName[typeName] = _tagIDGenerator++;
				_tagNames.push_back(typeName);
			}

			template<typename T>
			bool check_system_in_table()
			{
				std::string typeName = get_type_name<T>();
				if (_systemIDByName.find(typeName) != _systemIDByName.end())
					return true;
				return false;
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
				_systemNames.push_back(typeName);
			}

		private:
			struct ComponentInfo
			{
				uint32_t structureSize;
				uint32_t id;
			};

			std::unordered_map<std::string, ComponentInfo> _componentInfoByName;
			uint32_t _componentIDGenerator;
			std::unordered_map<std::string, uint32_t> _tagIDByName;
			std::vector<std::string> _tagNames;		// element index = type id
			uint32_t _tagIDGenerator;
			std::unordered_map<std::string, uint32_t> _systemIDByName;
			std::vector<std::string> _systemNames;   // element index = type id
			uint32_t _systemIDGenerator;
	};

	inline TypeInfoTable* TYPE_INFO_TABLE{ nullptr };

	inline void create_type_info_table() { TYPE_INFO_TABLE = new TypeInfoTable(); }
	inline void destroy_type_info_table() { delete TYPE_INFO_TABLE; }
	inline TypeInfoTable* get_active_type_info_table() { return TYPE_INFO_TABLE; }
	inline void set_type_info_table(TypeInfoTable* table) { TYPE_INFO_TABLE = table; }
}