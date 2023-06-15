#pragma once

#include "api.h"
#include "core/reflection.h"
#include "profiler/logger.h"

#include <unordered_map>
#include <mutex>

namespace ad_astris::ecs
{
	//Singleton, maybe I will change this class and make it static
	class ECS_API ComponentTypeIDTable
	{
		public:
			static ComponentTypeIDTable* get_instance()
			{
				std::lock_guard<std::mutex> lock(_mutex);
				if (!_instance)
					_instance = new ComponentTypeIDTable();
				return _instance;
			}
			
			template<typename T>
			bool check_in_table()
			{
				std::string typeName = get_type_name<T>();
				if (_table.find(typeName) != _table.end())
					return true;
				return false;
			}
	
			template<typename T>
			uint32_t get_type_id()
			{
				std::string typeName = get_type_name<T>();
				auto it = _table.find(typeName);
				if (it == _table.end())
					return -1;
	
				return it->second.id;
			}
	
			uint32_t get_type_id(std::string& typeName)
			{
				auto it = _table.find(typeName);
				if (it == _table.end())
					return -1;
	
				return it->second.id;
			}
	
			template<typename T>
			uint32_t get_type_size()
			{
				std::string typeName = get_type_name<T>();
				auto it = _table.find(typeName);
				if (it == _table.end())
					return -1;
	
				return it->second.structureSize;
			}
	
			uint32_t get_type_size(std::string& typeName)
			{
				auto it = _table.find(typeName);
				if (it == _table.end())
					return -1;
	
				return it->second.structureSize;
			}
	
			template<typename T>
			void add_component_info()
			{
				std::lock_guard<std::mutex> lock(_mutex);
				std::string typeName = get_type_name<T>();
				if (_table.find(typeName) != _table.end())
					return;
	
				ComponentInfo info;
				info.id = ++_idGenerator;
				info.structureSize = sizeof(T);
				_table[typeName] = info;
			}
			
		private:
			ComponentTypeIDTable() {}
			~ComponentTypeIDTable() {}
			
			struct ComponentInfo
			{
				uint32_t structureSize;
				uint32_t id;
			};
	
			static ComponentTypeIDTable* _instance;
			static std::mutex _mutex;
	
			static std::unordered_map<std::string, ComponentInfo> _table;
			static uint32_t _idGenerator;
	};
	
	// Returns the instance of the singleton ComponentTypeIDTable.
	inline ComponentTypeIDTable* get_type_id_table()
	{
		return ComponentTypeIDTable::get_instance();
	}

	class ECS_API TagTypeIdTable
	{
		public:
			template<typename T>
			static bool check_in_table()
			{
				std::string typeName = get_type_name<T>();
				if (_tableNameToId.find(typeName) != _tableNameToId.end())
					return true;
				return false;
			}
			
			template<typename T>
			static uint32_t get_type_id()
			{
				std::string typeName = get_type_name<T>();
				auto it = _tableNameToId.find(typeName);
				if (it == _tableNameToId.end())
					return -1;
			
				return it->second;
			}
			
			static uint32_t get_type_id(std::string& typeName)
			{
				auto it = _tableNameToId.find(typeName);
				if (it == _tableNameToId.end())
					return -1;
			
				return it->second;
			}

			static std::string get_tag_name(uint32_t typeID)
			{
				if (typeID >= _names.size())
				{
					LOG_ERROR("TagTypeIdTable::get_type_name(): Type ID > than names count")
					return "UndefinedTag";
				}
				return _names[typeID];
			}
			
			template<typename T>
			static uint32_t get_type_size()
			{
				std::string typeName = get_type_name<T>();
				auto it = _tableNameToId.find(typeName);
				if (it == _tableNameToId.end())
					return -1;
			
				return it->second;
			}
			
			template<typename T>
			static void add_tag()
			{
				std::string typeName = get_type_name<T>();
				if (_tableNameToId.find(typeName) != _tableNameToId.end())
					return;
					
				_tableNameToId[typeName] = _idGenerator++;
				_names.push_back(typeName);
			}
				
		private:
			static std::unordered_map<std::string, uint32_t> _tableNameToId;
			static std::vector<std::string> _names;   // element index = type id
			static uint32_t _idGenerator;
	};
	
	namespace tags
	{
		
	}
}
