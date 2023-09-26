#pragma once

#include "entity_types.h"

#include <json/json.hpp>

#include <unordered_map>
#include <mutex>
#include <cstdlib>

namespace ad_astris::ecs
{
	namespace serializers
	{
		class BaseSerializer
		{
			public:
				virtual void serialize(void* component, nlohmann::json& jsonForComponents) = 0;
				virtual void deserialize(EntityCreationContext& entityCreationContext, const std::string& componentInfo) = 0;
		};

		class SerializersTable
		{
			public:
				static SerializersTable* get_instance();
				
				template<typename COMPONENT_TYPE>
				void add_serializer(BaseSerializer* serializer)
				{
					uint32_t id = TYPE_INFO_TABLE->get_component_id<COMPONENT_TYPE>();
					std::lock_guard<std::mutex> lock(_mutex);
					_serializerByTypeID[id] = serializer;
				}
				
				BaseSerializer* get_serializer(uint32_t componentTypeID);
			
				bool has_serializer(uint32_t componentTypeID)
				{
					return _serializerByTypeID.find(componentTypeID) == _serializerByTypeID.end() ? false : true;
				}

			private:
				SerializersTable() {}
				~SerializersTable() {}
				
				static SerializersTable* _instance;
				static std::mutex _mutex;
				std::unordered_map<uint32_t, BaseSerializer*> _serializerByTypeID;
		};
		
		inline SerializersTable* get_table()
		{
			return SerializersTable::get_instance();
		}
	}
}