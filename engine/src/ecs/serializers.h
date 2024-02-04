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
		class ISerializer
		{
			public:
				virtual ~ISerializer() = default;
				virtual void serialize(void* component, nlohmann::json& jsonForComponents) = 0;
				virtual void deserialize(EntityCreationContext& entityCreationContext, nlohmann::json& componentInfo) = 0;
		};

		class SerializersTable
		{
			public:
				static SerializersTable* get_instance();
				
				template<typename COMPONENT_TYPE>
				void add_serializer(ISerializer* serializer)
				{
					uint64_t id = TypeInfoTable::get_component_id<COMPONENT_TYPE>();
					std::lock_guard<std::mutex> lock(_mutex);
					_serializerByTypeID[id] = serializer;
				}
				
				ISerializer* get_serializer(uint64_t componentTypeID);
			
				bool has_serializer(uint64_t componentTypeID)
				{
					return _serializerByTypeID.find(componentTypeID) == _serializerByTypeID.end() ? false : true;
				}

			private:
				SerializersTable() {}
				~SerializersTable() {}
				
				static SerializersTable* _instance;
				static std::mutex _mutex;
				std::unordered_map<uint64_t, ISerializer*> _serializerByTypeID;
		};
		
		inline SerializersTable* get_table()
		{
			return SerializersTable::get_instance();
		}
	}
}