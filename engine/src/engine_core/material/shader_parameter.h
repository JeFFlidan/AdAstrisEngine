#pragma once

#include "engine_core/uuid.h"
#include "core/math_base.h"
#include <variant>
#include <string>
#include <stdexcept>

namespace ad_astris::ecore
{
	enum class ShaderParameterType
	{
		UNDEFINED,
		BOOL,
		INT,
		UINT,
		FLOAT,
		UUID,
		FLOAT2,
		FLOAT3,
		FLOAT4,
		UINT2,
		UINT3,
		UINT4,
		INT2,
		INT3,
		INT4
	};
	
	class ShaderParameter
	{
		public:
			ShaderParameter(const std::string& name, bool value) : _name(name), _value(value) { }
			ShaderParameter(const std::string& name, int32_t value) : _name(name), _value(value) { }
			ShaderParameter(const std::string& name, uint32_t value) : _name(name), _value(value) { }
			ShaderParameter(const std::string& name, float value) : _name(name), _value(value) { }
			ShaderParameter(const std::string& name, UUID value) : _name(name), _value(value) { }
			ShaderParameter(const std::string& name, const XMFLOAT2& value) : _name(name), _value(value) { }
			ShaderParameter(const std::string& name, const XMFLOAT3& value) : _name(name), _value(value) { }
			ShaderParameter(const std::string& name, const XMFLOAT4& value) : _name(name), _value(value) { }
			ShaderParameter(const std::string& name, const XMINT2& value) : _name(name), _value(value) { }
			ShaderParameter(const std::string& name, const XMINT3& value) : _name(name), _value(value) { }
			ShaderParameter(const std::string& name, const XMINT4& value) : _name(name), _value(value) { }
			ShaderParameter(const std::string& name, const XMUINT2& value) : _name(name), _value(value) { }
			ShaderParameter(const std::string& name, const XMUINT3& value) : _name(name), _value(value) { }
			ShaderParameter(const std::string& name, const XMUINT4& value) : _name(name), _value(value) { }
		
			ShaderParameterType get_type() const
			{
				return std::visit([&](auto&& arg)->ShaderParameterType
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, bool>)
						return ShaderParameterType::BOOL;
					if constexpr (std::is_same_v<ArgType, int32_t>)
						return ShaderParameterType::INT;
					if constexpr (std::is_same_v<ArgType, uint32_t>)
						return ShaderParameterType::UINT;
					if constexpr (std::is_same_v<ArgType, float>)
						return ShaderParameterType::FLOAT;
					if constexpr (std::is_same_v<ArgType, UUID>)
						return ShaderParameterType::UUID;
					if constexpr (std::is_same_v<ArgType, XMFLOAT2>)
						return ShaderParameterType::FLOAT2;
					if constexpr (std::is_same_v<ArgType, XMFLOAT3>)
						return ShaderParameterType::FLOAT3;
					if constexpr (std::is_same_v<ArgType, XMFLOAT4>)
						return ShaderParameterType::FLOAT4;
					if constexpr (std::is_same_v<ArgType, XMUINT2>)
						return ShaderParameterType::UINT2;
					if constexpr (std::is_same_v<ArgType, XMUINT3>)
						return ShaderParameterType::UINT3;
					if constexpr (std::is_same_v<ArgType, XMUINT4>)
						return ShaderParameterType::UINT4;
					if constexpr (std::is_same_v<ArgType, XMINT2>)
						return ShaderParameterType::INT2;
					if constexpr (std::is_same_v<ArgType, XMINT3>)
						return ShaderParameterType::INT3;
					if constexpr (std::is_same_v<ArgType, XMINT4>)
						return ShaderParameterType::INT4;
					return ShaderParameterType::UNDEFINED;
				}, _value);
			}

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
			operator ValueType() const
			{
				return get<ValueType>();
			}

			template<typename T>
			T get() const { throw std::runtime_error("ShaderParameter::get(): Unimplemented"); }

			template<>
			bool get() const
			{
				if (is_valid<ShaderParameterType::BOOL>())
					return std::get<bool>(_value);
				return false;
			}

			template<>
			int32_t get() const
			{
				if (is_valid<ShaderParameterType::INT>())
					return std::get<int32_t>(_value);
				return 0;
			}

			template<>
			uint32_t get() const
			{
				if (is_valid<ShaderParameterType::UINT>())
					return std::get<uint32_t>(_value);
				return 0;
			}
		
			template<>
			float get() const
			{
				if (is_valid<ShaderParameterType::FLOAT>())
					return std::get<float>(_value);
				return 0;
			}

			template<>
			UUID get() const
			{
				if (is_valid<ShaderParameterType::UUID>())
					return std::get<UUID>(_value);
				return 0;
			}

			template<>
			XMFLOAT2 get() const
			{
				if (is_valid<ShaderParameterType::FLOAT2>())
					return std::get<XMFLOAT2>(_value);
				return { 0, 0 };
			}

			template<>
			XMFLOAT3 get() const
			{
				if (is_valid<ShaderParameterType::FLOAT3>())
					return std::get<XMFLOAT3>(_value);
				return { 0, 0, 0 };
			}

			template<>
			XMFLOAT4 get() const
			{
				if (is_valid<ShaderParameterType::FLOAT4>())
					return std::get<XMFLOAT4>(_value);
				return { 0, 0, 0, 0 };
			}
	
			template<>
			XMINT2 get() const
			{
				if (is_valid<ShaderParameterType::INT2>())
					return std::get<XMINT2>(_value);
				return { 0, 0 };
			}

			template<>
			XMINT3 get() const
			{
				if (is_valid<ShaderParameterType::INT3>())
					return std::get<XMINT3>(_value);
				return { 0, 0, 0 };
			}

			template<>
			XMINT4 get() const
			{
				if (is_valid<ShaderParameterType::INT4>())
					return std::get<XMINT4>(_value);
				return { 0, 0, 0, 0 };
			}

			template<>
			XMUINT2 get() const
			{
				if (is_valid<ShaderParameterType::UINT2>())
					return std::get<XMUINT2>(_value);
				return { 0, 0 };
			}

			template<>
			XMUINT3 get() const
			{
				if (is_valid<ShaderParameterType::UINT3>())
					return std::get<XMUINT3>(_value);
				return { 0, 0, 0 };
			}

			template<>
			XMUINT4 get() const
			{
				if (is_valid<ShaderParameterType::UINT4>())
					return std::get<XMUINT4>(_value);
				return { 0, 0, 0, 0 };
			}

			ShaderParameter& operator=(bool value)
			{
				if (is_valid<ShaderParameterType::BOOL>())
					_value = value;
				return *this;
			}

			ShaderParameter& operator=(int32_t value)
			{
				if (is_valid<ShaderParameterType::INT>())
					_value = value;
				return *this;
			}

			ShaderParameter& operator=(uint32_t value)
			{
				if (is_valid<ShaderParameterType::UINT>())
					_value = value;
				return *this;
			}

			ShaderParameter& operator=(float value)
			{
				if (is_valid<ShaderParameterType::FLOAT>())
					_value = value;
				return *this;
			}

			ShaderParameter& operator=(UUID value)
			{
				if (is_valid<ShaderParameterType::UUID>())
					_value = value;
				return *this;
			}

			ShaderParameter& operator=(XMFLOAT2 value)
			{
				if (is_valid<ShaderParameterType::FLOAT2>())
					_value = value;
				return *this;
			}

			ShaderParameter& operator=(const XMFLOAT3& value)
			{
				if (is_valid<ShaderParameterType::FLOAT3>())
					_value = value;
				return *this;
			}

			ShaderParameter& operator=(const XMFLOAT4& value)
			{
				if (is_valid<ShaderParameterType::FLOAT4>())
					_value = value;
				return *this;
			}

			ShaderParameter& operator=(XMINT2 value)
			{
				if (is_valid<ShaderParameterType::INT2>())
					_value = value;
				return *this;
			}

			ShaderParameter& operator=(const XMINT3& value)
			{
				if (is_valid<ShaderParameterType::INT3>())
					_value = value;
				return *this;
			}

			ShaderParameter& operator=(const XMINT4& value)
			{
				if (is_valid<ShaderParameterType::INT4>())
					_value = value;
				return *this;
			}

			ShaderParameter& operator=(XMUINT2 value)
			{
				if (is_valid<ShaderParameterType::UINT2>())
					_value = value;
				return *this;
			}

			ShaderParameter& operator=(const XMUINT3& value)
			{
				if (is_valid<ShaderParameterType::UINT3>())
					_value = value;
				return *this;
			}
		
			ShaderParameter& operator=(const XMUINT4& value)
			{
				if (is_valid<ShaderParameterType::UINT4>())
					_value = value;
				return *this;
			}

		private:
			std::string _name;
			std::variant<bool, int32_t, uint32_t, float, UUID, XMFLOAT2, XMFLOAT3, XMFLOAT4, XMUINT2, XMUINT3, XMUINT4, XMINT2, XMINT3, XMINT4> _value;

			template<ShaderParameterType ParameterType>
			bool is_valid() const
			{
				if (get_type() == ParameterType)
					return true;
				LOG_ERROR("ShaderParameterType::is_valid(): Passed parameter type is invalid")
				return false;
			}
	};
}