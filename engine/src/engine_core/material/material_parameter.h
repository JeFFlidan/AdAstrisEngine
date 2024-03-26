#pragma once

#include "material_parameter_types.h"
#include "material_parameter_metadata.h"
#include "engine_core/uuid.h"
#include "core/math_base.h"
#include <variant>
#include <string>
#include <cassert>

namespace ad_astris::ecore
{
	//Must test performance of methods with FORCE_INLINE
	class MaterialParameter
	{
		public:
			MaterialParameter(MaterialParameterMetadata* metadata) : _metadata(metadata), _value(metadata->get_default_value()) { }
			MaterialParameter(MaterialParameterMetadata* metadata, bool value) : _metadata(metadata), _value(value) { }
			MaterialParameter(MaterialParameterMetadata* metadata, int32_t value) : _metadata(metadata), _value(value) { }
			MaterialParameter(MaterialParameterMetadata* metadata, uint32_t value) : _metadata(metadata), _value(value) { }
			MaterialParameter(MaterialParameterMetadata* metadata, float value) : _metadata(metadata), _value(value) { }
			MaterialParameter(MaterialParameterMetadata* metadata, UUID value) : _metadata(metadata), _value(value) { }
			MaterialParameter(MaterialParameterMetadata* metadata, const XMFLOAT2& value) : _metadata(metadata), _value(value) { }
			MaterialParameter(MaterialParameterMetadata* metadata, const XMFLOAT3& value) : _metadata(metadata), _value(value) { }
			MaterialParameter(MaterialParameterMetadata* metadata, const XMFLOAT4& value) : _metadata(metadata), _value(value) { }
			MaterialParameter(MaterialParameterMetadata* metadata, const XMINT2& value) : _metadata(metadata), _value(value) { }
			MaterialParameter(MaterialParameterMetadata* metadata, const XMINT3& value) : _metadata(metadata), _value(value) { }
			MaterialParameter(MaterialParameterMetadata* metadata, const XMINT4& value) : _metadata(metadata), _value(value) { }
			MaterialParameter(MaterialParameterMetadata* metadata, const XMUINT2& value) : _metadata(metadata), _value(value) { }
			MaterialParameter(MaterialParameterMetadata* metadata, const XMUINT3& value) : _metadata(metadata), _value(value) { }
			MaterialParameter(MaterialParameterMetadata* metadata, const XMUINT4& value) : _metadata(metadata), _value(value) { }

			const MaterialParameterMetadata* get_metadata() const { return _metadata; }
		
			template<typename ValueType, typename std::enable_if<
				std::disjunction<
					std::is_same<bool, ValueType>,
					std::is_same<int32_t, ValueType>,
					std::is_same<uint32_t, ValueType>,
					std::is_same<float, ValueType>,
					std::is_same<UUID, ValueType>,
					std::is_same<XMFLOAT2, ValueType>,
					std::is_same<XMFLOAT3, ValueType>,
					std::is_same<XMFLOAT4, ValueType>,
					std::is_same<XMINT2, ValueType>,
					std::is_same<XMINT3, ValueType>,
					std::is_same<XMINT4, ValueType>,
					std::is_same<XMUINT2, ValueType>,
					std::is_same<XMUINT3, ValueType>,
					std::is_same<XMUINT4, ValueType>>::value, int>::type = 0>
			FORCE_INLINE operator ValueType() const
			{
				return get<ValueType>();
			}

			template<typename T>
			FORCE_INLINE T get() const { throw std::runtime_error("ShaderParameter::get(): Unimplemented"); }

			template<>
			FORCE_INLINE bool get() const
			{
				assert(has_value<bool>());
				return std::get<bool>(_value);
			}

			template<>
			FORCE_INLINE int32_t get() const
			{
				assert(has_value<int32_t>());
				return std::get<int32_t>(_value);
			}

			template<>
			FORCE_INLINE uint32_t get() const
			{
				assert(has_value<uint32_t>());
				return std::get<uint32_t>(_value);
			}
		
			template<>
			FORCE_INLINE float get() const
			{
				assert(has_value<float>());
				return std::get<float>(_value);
			}

			template<>
			FORCE_INLINE UUID get() const
			{
				assert(has_value<UUID>());
				return std::get<UUID>(_value);
			}

			template<>
			FORCE_INLINE XMFLOAT2 get() const
			{
				assert(has_value<XMFLOAT2>());
				return std::get<XMFLOAT2>(_value);
			}

			template<>
			FORCE_INLINE XMFLOAT3 get() const
			{
				assert(has_value<XMFLOAT3>());
				return std::get<XMFLOAT3>(_value);
			}

			template<>
			FORCE_INLINE XMFLOAT4 get() const
			{
				assert(has_value<XMFLOAT4>());
				return std::get<XMFLOAT4>(_value);
			}
	
			template<>
			FORCE_INLINE XMINT2 get() const
			{
				assert(has_value<XMINT2>());
				return std::get<XMINT2>(_value);
			}

			template<>
			FORCE_INLINE XMINT3 get() const
			{
				assert(has_value<XMINT3>());
				return std::get<XMINT3>(_value);
			}

			template<>
			FORCE_INLINE XMINT4 get() const
			{
				assert(has_value<XMINT4>());
				return std::get<XMINT4>(_value);
			}

			template<>
			FORCE_INLINE XMUINT2 get() const
			{
				assert(has_value<XMUINT2>());
				return std::get<XMUINT2>(_value);
			}

			template<>
			FORCE_INLINE XMUINT3 get() const
			{
				assert(has_value<XMUINT3>());
				return std::get<XMUINT3>(_value);
			}

			template<>
			FORCE_INLINE XMUINT4 get() const
			{
				assert(has_value<XMUINT4>());
				return std::get<XMUINT4>(_value);
			}

			FORCE_INLINE const void* get_ptr() const
			{
				return std::visit([&](auto& arg)->const void*
				{
					return &arg;
				}, _value);
			}

			FORCE_INLINE MaterialParameter& operator=(bool value)
			{
				assert(has_value<bool>());
				_value = value;
				return *this;
			}

			FORCE_INLINE MaterialParameter& operator=(int32_t value)
			{
				assert(has_value<int32_t>());
				_value = value;
				return *this;
			}

			FORCE_INLINE MaterialParameter& operator=(uint32_t value)
			{
				assert(has_value<uint32_t>());
				_value = value;
				return *this;
			}

			FORCE_INLINE MaterialParameter& operator=(float value)
			{
				assert(has_value<float>());
				_value = value;
				return *this;
			}

			FORCE_INLINE MaterialParameter& operator=(UUID value)
			{
				assert(has_value<UUID>());
				_value = value;
				return *this;
			}

			FORCE_INLINE MaterialParameter& operator=(XMFLOAT2 value)
			{
				assert(has_value<XMFLOAT2>());
				_value = value;
				return *this;
			}

			FORCE_INLINE MaterialParameter& operator=(const XMFLOAT3& value)
			{
				assert(has_value<XMFLOAT3>());
				_value = value;
				return *this;
			}

			FORCE_INLINE MaterialParameter& operator=(const XMFLOAT4& value)
			{
				assert(has_value<XMFLOAT4>());
				_value = value;
				return *this;
			}

			FORCE_INLINE MaterialParameter& operator=(XMINT2 value)
			{
				assert(has_value<XMINT2>());
				_value = value;
				return *this;
			}

			FORCE_INLINE MaterialParameter& operator=(const XMINT3& value)
			{
				assert(has_value<XMINT3>());
				_value = value;
				return *this;
			}

			FORCE_INLINE MaterialParameter& operator=(const XMINT4& value)
			{
				assert(has_value<XMINT4>());
				_value = value;
				return *this;
			}

			FORCE_INLINE MaterialParameter& operator=(XMUINT2 value)
			{
				assert(has_value<XMUINT2>());
				_value = value;
				return *this;
			}

			FORCE_INLINE MaterialParameter& operator=(const XMUINT3& value)
			{
				assert(has_value<XMUINT3>());
				_value = value;
				return *this;
			}
		
			FORCE_INLINE MaterialParameter& operator=(const XMUINT4& value)
			{
				assert(has_value<XMUINT4>());
				_value = value;
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