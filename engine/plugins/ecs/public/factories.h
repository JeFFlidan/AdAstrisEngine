#pragma once

#include "entity_types.h"
#include "serializers.h"

#include <json.hpp>

#include <unordered_map>
#include <mutex>
#include <tuple>
#include <cstdlib>

namespace ad_astris::ecs
{
	namespace factories
	{
		class ECS_API BaseFactory
		{
			public:
				void build(
					EntityCreationContext& creationContext,
					uint32_t componentTypeId,
					std::string& componentName,
					nlohmann::json& jsonWithComponents);

			protected:
				virtual void build_object(
					EntityCreationContext& creationContext,
					std::string& componentName,
					nlohmann::json& jsonWithComponents,
					serializers::BaseSerializer* serializer) = 0;
		};

		template<typename T, typename ...ARGS>
		T copy_data(ARGS... args)
		{
			T newObj{};
			new(&newObj) T{ std::forward<ARGS>(args)... };
			return newObj;
		}

		// I've decided to use singleton here, but I have to think if it is a good idea
		class ECS_API FactoriesTable
		{
			public:
				static FactoriesTable* get_instance();
			
				template<typename COMPONENT_TYPE>
				void add_factory(BaseFactory* factory)
				{
					uint32_t id = TypeInfoTable::get_component_id<COMPONENT_TYPE>();
					std::lock_guard<std::mutex> lock(_mutex);
					_typeIdToFactory[id] = factory;
				}
			
				BaseFactory* get_factory(uint32_t componentTypeId);

			private:
				FactoriesTable() {}
				~FactoriesTable() {}
			
				static FactoriesTable* _instance;
				static std::mutex _mutex;
				std::unordered_map<uint32_t, BaseFactory*> _typeIdToFactory;
		};

		inline FactoriesTable* get_table()
		{
			return FactoriesTable::get_instance();
		}
	}
}
