/* ====================================================
	This file must be included in the main program if
	you want to use the Entity-Component-System module.
	It consists of some classes that can't be implemented
	in the dll source files because of its limitations.
	Also, this file contains macro related to Entity-Component-System.
   ======================================================*/

#pragma once

#include "core/glm_to_json.h"
#include "public/api.h"
#include "public/archetype_types.h"
#include "public/archetype.h"
#include "public/entity_manager.h"
#include "public/entity_types.h"
#include "public/factories.h"
#include "public/serializers.h"
#include "public/system_manager.h"
#include "core/utils.h"

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
 				_size = ComponentTypeIDTable::get_type_size<T>();
 				_id = ComponentTypeIDTable::get_type_id<T>();
 			}
 		
			T* get_memory()
			{
				return static_cast<T*>(_memory);
			}
	};
}

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
#define ECS_COMPONENT(Type, ...)																			\
	namespace ecs																							\
	{																										\
		namespace factories																					\
		{																									\
			class Type##Factory : public BaseFactory														\
			{																								\
				protected:																					\
					void build_object(																		\
						EntityCreationContext& creationContext,												\
						std::string& componentName,															\
						nlohmann::json& jsonWithComponents,													\
						serializers::BaseSerializer* serializer) override									\
					{																						\
						void* tupleVoid = serializer->deserialize(componentName, jsonWithComponents);		\
						std::tuple<__VA_ARGS__>* tuplePtr = static_cast<std::tuple<__VA_ARGS__>*>(tupleVoid);\
						Type newObj = std::apply(copy_data<Type, __VA_ARGS__>, *tuplePtr);					\
						creationContext.add_component<Type>(newObj);										\
						delete tuplePtr;																	\
					}																						\
			};																								\
																											\
		}																									\
																											\
		namespace serializers																				\
		{																									\
			class Type##Serializer : public BaseSerializer													\
			{																								\
				public:																						\
					void serialize(void* component, nlohmann::json& jsonForComponents) override				\
					{																						\
						serialize_internal(component, jsonForComponents);									\
					}																						\
																											\
					void* deserialize(																		\
						std::string& componentName,															\
						nlohmann::json& jsonWithComponents) override										\
					{																						\
						std::tuple<__VA_ARGS__>* tuplePtr = new std::tuple<__VA_ARGS__>();					\
						std::tuple<__VA_ARGS__> tempTuple = jsonWithComponents[componentName]				\
							.get<std::tuple<__VA_ARGS__>>();												\
						*tuplePtr = tempTuple;																\
						return tuplePtr;																	\
					}																						\
				private:																					\
					template<typename ...ARGS>																\
					void serialize_internal(void* component, nlohmann::json& jsonForComponents)				\
					{																						\
						Type* typedComponent = static_cast<Type*>(component);								\
						constexpr size_t argCount = CoreUtils::count_args<__VA_ARGS__>();					\
						auto tuple = CoreUtils::custom_data_type_to_tuple<argCount>(*typedComponent);		\
						std::string typeName = get_type_name<Type>();										\
						jsonForComponents[typeName] = tuple;												\
					}																						\
			};																								\
																											\
			static bool Type##Register = []()																\
			{																								\
				ComponentTypeIDTable::add_component_info<Type>();											\
				factories::BaseFactory* factory = new factories::Type##Factory();							\
				factories::FactoriesTable::get_instance()->add_factory<Type>(factory);						\
				BaseSerializer* serializer = new Type##Serializer();										\
				serializers::get_table()->add_serializer<Type>(serializer);									\
				return true;																				\
			}();																							\
		}																									\
	}


#define ECS_TAG(Type)										\
namespace ecs {												\
namespace tags {											\
	static bool Type##Register = []()						\
	{														\
		::ad_astris::ecs::TagTypeIDTable::add_tag<Type>();	\
		return true;										\
	}();													\
}}

#define ECS_SYSTEM(Type)																		\
	namespace ecs {																				\
	namespace systems {																			\
		static bool Type##Result = SystemStorage::SystemRegistrar<Type>::register_system();		\
	}}