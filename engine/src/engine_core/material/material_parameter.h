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
			NLOHMANN_DEFINE_TYPE_INTRUSIVE(MaterialParameter, _value)
		
			MaterialParameter(MaterialParameterMetadata* metadata) : _metadata(metadata), _value(metadata->get_default_value()) { }
			MaterialParameter(MaterialParameterMetadata* metadata, MaterialParameterValue value) : _metadata(metadata), _value(value) { }
			MaterialParameter(MaterialParameterMetadata* metadata, bool value) : _metadata(metadata), _value(MaterialBool{ (uint32_t)value} ) { }

			const MaterialParameterMetadata* get_metadata() const { return _metadata; }
		
			template<typename ValueType, typename std::enable_if<
				internal::MaterialParameterTypesPack::has_type<ValueType>()
				|| std::is_same_v<ValueType, MaterialParameterValue>, int>::type = 0>
			FORCE_INLINE operator ValueType() const
			{
				return get<ValueType>();
			}

			template<typename ValueType, typename std::enable_if<
				internal::MaterialParameterTypesPack::has_type<ValueType>()
				|| std::is_same_v<ValueType, MaterialParameterValue>, int>::type = 0>
			FORCE_INLINE ValueType get() const
			{
				if constexpr (internal::MaterialParameterTypesPack::has_type<ValueType>())
				{
					assert(has_value<ValueType>());
					return std::get<ValueType>(_value);
				}
				return _value;
			}

			FORCE_INLINE const void* get_ptr() const
			{
				return std::visit([&](auto& arg)->const void*
				{
					return &arg;
				}, _value);
			}
		
			FORCE_INLINE MaterialParameter& operator=(MaterialParameterValue value)
			{
				assert((MaterialParameterType)value.index() == _metadata->get_type());
				_value = value;
				return *this;
			}

			FORCE_INLINE MaterialParameter& operator=(bool value)
			{
				assert(has_value<MaterialBool>());
				_value = MaterialBool{ (uint32_t)value };
				return *this;
			}

			void reset_to_default_value() { _value = _metadata->get_default_value(); }

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