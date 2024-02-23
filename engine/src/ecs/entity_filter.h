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
			virtual bool validate_by_any(Entity entity) const = 0;
			virtual bool is_equal(const IEntityFilter& filter) const = 0;
			virtual std::string get_name() const = 0;
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
				if (!entity.is_valid())
				{
					LOG_ERROR("EntityFilter::validate(): Entity {} is invalid", entity)
					return false;
				}
				
				bool result = true;
				(validate_internal<Requirements>(entity, result), ...);
				return result;
			}

			bool validate_by_any(Entity entity) const override
			{
				if (!entity.is_valid())
				{
					LOG_ERROR("EntityFilter::validate_by_any(): Entity {} is invalid", entity)
					return false;
				}
				
				bool result = false;
				(validate_by_any<Requirements>(entity, result), ...);
				return result;
			}

			bool is_equal(const IEntityFilter& filter) const override
			{
				return _hash == filter.get_requirements_hash();
			}

			std::string get_name() const override
			{
				std::string name;
				(generate_name<Requirements>(name), ...);
				return name;
			}

			template<typename Requirement>
			static constexpr bool has_requirement()
			{
				return (std::is_same_v<Requirement, Requirements> || ...);
			}

		private:
			template<typename T>
			void validate_internal(Entity entity, bool& result) const
			{
				if (!entity.has_property<T>())
				{
					result = false;
				}
			}

			template<typename T>
			void validate_by_any(Entity entity, bool& result) const
			{
				if (entity.has_property<T>())
				{
					result = true;
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

			template<typename T>
			void generate_name(std::string& outName) const
			{
				outName += get_type_name<T>();
			}
	};
}