/** This file consists classes which are used in the dll and the main programme, 
 * however they can't be implemented in the dll source files because of its limitations
 * Maybe, I will remove this file and return all classes into dll source.
 */

#pragma once

#include "public/api.h"
#include "public/archetype_types.h"
#include "public/archetype.h"
#include "public/entity_system.h"
#include "public/entity_types.h"
#include "public/factories.h"
#include "public/serializers.h"
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
 				_size = get_type_id_table()->get_type_size<T>();
 				_id = get_type_id_table()->get_type_id<T>();
 			}
 		
			T* get_memory()
			{
				return static_cast<T*>(_memory);
			}
	};
}

#define COMPONENT(Type, ...)																				\
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
			static bool Type##FactoryRegistered = []()														\
			{																								\
				BaseFactory* factory = new Type##Factory();													\
				FactoriesTable::get_instance()->add_factory<Type>(factory);									\
				return true;																				\
			}();																							\
																											\
			static bool Type##Register = []()																\
			{																								\
				get_type_id_table()->add_component_info<Type>();											\
				return true;																				\
			}();																							\
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
			static bool Type##SerializerRegistered = []()													\
			{																								\
				BaseSerializer* serializer = new Type##Serializer();										\
				serializers::get_table()->add_serializer<Type>(serializer);									\
				return true;																				\
			}();																							\
		}																									\
	}