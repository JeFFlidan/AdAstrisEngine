#pragma once

#include "entity_types.h"
#include "attributes.h"
#include "core/global_objects.h"
#include "core/reflection.h"
#include "core/utils.h"
#include <vector>

namespace ad_astris::ecs
{
	class IEntityFilter
	{
		public:
			virtual ~IEntityFilter() = default;
			virtual bool validate(Entity entity) const = 0;
			virtual bool is_equal(const IEntityFilter& filter) const = 0;
			size_t get_requirements_hash() const { return _hash; }

		protected:
			size_t _hash;
	};
	
	template<typename ...Requirements>
	class EntityFilter : public IEntityFilter
	{
		public:
			EntityFilter()
			{
				size_t index = 0;
				std::vector<uint64_t> ids(CoreUtils::count_args<Requirements...>());
				(fill_ids_array<Requirements>(ids, index), ...);
				std::sort(ids.begin(), ids.end());
				_hash = CoreUtils::hash_numeric_vector(ids);
			}
		
			bool validate(Entity entity) const override
			{
				bool result = true;
				(validate_internal<Requirements>(entity, result), ...);
				return result;
			}

			bool is_equal(const IEntityFilter& filter) const override
			{
				return _hash == filter.get_requirements_hash();
			}

		private:
			template<typename T>
			void validate_internal(Entity entity, bool& result) const
			{
				if constexpr (Reflector::has_attribute<T, EcsComponent>())
				{
					if (!WORLD()->get_entity_manager()->does_entity_have_component<T>(entity))
					{
						result = false;
					}
				}
				if constexpr (Reflector::has_attribute<T, EcsTag>())
				{
					if (!WORLD()->get_entity_manager()->does_entity_have_tag<T>(entity))
					{
						result = false;
					}
				}
			}

			template<typename T>
			void fill_ids_array(std::vector<uint64_t>& ids, size_t& index)
			{
				if constexpr (Reflector::has_attribute<T, EcsComponent>())
				{
					ids[index++] = TypeInfoTable::get_component_id<T>();
				}
				if constexpr (Reflector::has_attribute<T, EcsTag>())
				{
					ids[index++] = TypeInfoTable::get_tag_id<T>();
				}
			}
	};
}