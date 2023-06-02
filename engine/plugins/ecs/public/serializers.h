#pragma once

#include "entity_types.h"

#include <json.hpp>

#include <unordered_map>
#include <mutex>
#include <cstdlib>

namespace ad_astris::ecs
{
	namespace serializers
	{
		class ECS_API BaseSerializer
		{
			public:
				virtual void serialize(void* component, nlohmann::json& jsonForComponents) = 0;
				virtual void* deserialize(std::string& componentName, nlohmann::json& jsonWithComponents) = 0;
		};

		class ECS_API SerializersTable
		{
			public:
				static SerializersTable* get_instance();
				
				template<typename COMPONENT_TYPE>
				void add_serializer(BaseSerializer* serializer)
				{
					uint32_t id = get_type_id_table()->get_type_id<COMPONENT_TYPE>();
					std::lock_guard<std::mutex> lock(_mutex);
					_typeIdToFactory[id] = serializer;
				}
				
				BaseSerializer* get_serializer(uint32_t componentTypeId);

			private:
				SerializersTable() {}
				~SerializersTable() {}
				
				static SerializersTable* _instance;
				static std::mutex _mutex;
				std::unordered_map<uint32_t, BaseSerializer*> _typeIdToFactory;
		};
		
		inline SerializersTable* get_table()
		{
			return SerializersTable::get_instance();
		}
	}
}