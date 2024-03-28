#pragma once

#include "material_parameter_types.h"
#include "material_parameter_metadata.h"
#include "core/common.h"
#include <variant>
#include <cassert>

namespace ad_astris::ecore
{
	//Must test performance of methods with FORCE_INLINE
	class MaterialParameter
	{
		public:
			MaterialParameter(MaterialParameterMetadata* metadata) : _metadata(metadata), _value(metadata->get_default_value()) { }

			template<typename ValueType, typename std::enable_if<
				internal::MaterialParameterTypesPack::has_type<ValueType>(), int>::type = 0>
			MaterialParameter(MaterialParameterMetadata* metadata, ValueType value) : _metadata(metadata), _value(value) { }
		
			MaterialParameter(MaterialParameterMetadata* metadata, bool value) : _metadata(metadata), _value(MaterialBool{ (uint32_t)value} ) { }

			const MaterialParameterMetadata* get_metadata() const { return _metadata; }
		
			template<typename ValueType, typename std::enable_if<
				internal::MaterialParameterTypesPack::has_type<ValueType>(), int>::type = 0>
			FORCE_INLINE operator ValueType() const
			{
				return get<ValueType>();
			}

			template<typename ValueType, typename std::enable_if<
				internal::MaterialParameterTypesPack::has_type<ValueType>(), int>::type = 0>
			FORCE_INLINE ValueType get() const
			{
				assert(has_value<ValueType>());
				return std::get<ValueType>(_value);
			}

			FORCE_INLINE const void* get_ptr() const
			{
				return std::visit([&](auto& arg)->const void*
				{
					return &arg;
				}, _value);
			}

			template<typename ValueType, typename std::enable_if<
				internal::MaterialParameterTypesPack::has_type<ValueType>(), int>::type = 0>
			FORCE_INLINE MaterialParameter& operator=(ValueType value)
			{
				assert(has_value<ValueType>());
				_value = value;
				return *this;
			}

			FORCE_INLINE MaterialParameter& operator=(bool value)
			{
				assert(has_value<MaterialBool>());
				_value = MaterialBool{ (uint32_t)value };
				return *this;
			}

		private:
			MaterialParameterMetadata* _metadata{ nullptr };
			MaterialParameterValue _value;

			template<typename ValueType>
			bool has_value() const
			{
				if (std::holds_alternative<ValueType>(_value))
					return true;
				return false;
			}
	};
}