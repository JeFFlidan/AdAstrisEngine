/* ====================================================
	This file must be included in the main program if
	you want to use the Entity-Component-System module.
	It consists of some classes that can't be implemented
	in the dll source files because of its limitations.
	Also, this file contains macro related to Entity-Component-System.
   ======================================================*/

#pragma once

#include "entity_types.h"
#include "serializers.h"
#include "system_manager.h"
#include "core/serialization.h"
#include "core/reflection.h"

namespace ad_astris::ecs
{
 	template<typename T>
	class Component : public UntypedComponent
	{
		public:
			Component() = default;
 		
 			Component(T* componentValue)
 			{
 				_memory = componentValue;
 				_size = TypeInfoTable::get_component_size<T>();
 				_id = TypeInfoTable::get_component_id<T>();
 			}
 		
			T* get_memory()
			{
				return static_cast<T*>(_memory);
			}
	};
}

#define H1(s,i,x)   (x*65599u+(uint8_t)s[(i)<strlen(s)?strlen(s)-1-(i):strlen(s)])
#define H4(s,i,x)   H1(s,i,H1(s,i+1,H1(s,i+2,H1(s,i+3,x))))
#define H16(s,i,x)  H4(s,i,H4(s,i+4,H4(s,i+8,H4(s,i+12,x))))
#define H64(s,i,x)  H16(s,i,H16(s,i+16,H16(s,i+32,H16(s,i+48,x))))
#define H256(s,i,x) H64(s,i,H64(s,i+64,H64(s,i+128,H64(s,i+192,x))))

#define HASH(s)    ((uint32_t)(H256(s,0,0)^(H256(s,0,0)>>16)))

/**
 * @brief COMPONENT Macro
 *
 * This macro creates a factory named TypeFactory
 * (where Type is the specified component type) and a serializer class
 * TypeSerializer responsible for handling serialization and
 * deserialization operations. Both classes inherit from base classes.
 * Additionally, the macro adds the newly created factory and serializer
 * objects to their respective collections.
 * 
 * WITHOUT THIS MACRO, A COMPONENT CANNOT BE ADDED TO AN ENTITY, SERIALIZED AND DESERIALIZED.
 * 
 * Usage example:
 * COMPONENT(Velocity, float, float, glm::vec3)
 * 
 * @param Type The name of the component type.
 * @param Fields The data types of the component fields.
 */
#define REGISTER_COMPONENT(Type, TypeWithNamespace, ...)															\
	REFLECT_SERIALIZABLE_FIELDS(TypeWithNamespace, __VA_ARGS__)											\
	namespace ad_astris::ecs																				\
	{																										\
		namespace																							\
		{																									\
			class Type##Serializer : public serializers::BaseSerializer										\
			{																								\
				public:																						\
					void serialize(void* component, nlohmann::json& jsonForComponents) override				\
					{																						\
						TypeWithNamespace* typedComponent = static_cast<TypeWithNamespace*>(component);		\
						std::string typeName = get_type_name<TypeWithNamespace>();							\
						jsonForComponents[typeName] = serialization::serialize_to_json(*typedComponent);	\
					}																						\
																											\
					void deserialize(																		\
						EntityCreationContext& entityCreationContext,										\
						const std::string& componentInfo) override												\
					{																						\
						TypeWithNamespace component;														\
						serialization::deserialize_from_json(componentInfo, component);						\
						entityCreationContext.add_component(component);										\
					}																						\
			};																								\
			static bool Type##Register = []()																\
			{																								\
				TypeInfoTable::add_component_info<TypeWithNamespace>();										\
				serializers::BaseSerializer* serializer = new Type##Serializer();							\
				serializers::get_table()->add_serializer<TypeWithNamespace>(serializer);					\
				return true;																				\
			}();																							\
		}																									\
	}


#define REGISTER_TAG(Type, TypeWithNamespace)				\
namespace ad_astris::ecs {									\
namespace {													\
	static bool Type##Register = []()						\
	{														\
		TypeInfoTable::add_tag<TypeWithNamespace>();		\
		return true;										\
	}();													\
}}

#define REGISTER_SYSTEM(Type, TypeWithNamespace)																					\
	namespace ad_astris::ecs {																								\
	namespace {																												\
		static bool Type##Register = registration::SystemStorage::SystemRegistrar<TypeWithNamespace>::register_system();	\
	}}