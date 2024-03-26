#pragma once

#include "core/math_base.h"
#include "core/template_utils.h"
#include <variant>

namespace ad_astris::ecore
{
	enum class MaterialParameterType
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

	namespace material_utils
	{
		inline std::map<std::type_index, MaterialParameterType> MATERIAL_PARAMETER_TYPE_BY_INDEX = {
			{ std::type_index(typeid(bool)), MaterialParameterType::BOOL },
			{ std::type_index(typeid(int32_t)), MaterialParameterType::INT },
			{ std::type_index(typeid(uint32_t)), MaterialParameterType::UINT },
			{ std::type_index(typeid(float)), MaterialParameterType::FLOAT },
			{ std::type_index(typeid(UUID)), MaterialParameterType::UUID },
			{ std::type_index(typeid(XMFLOAT2)), MaterialParameterType::FLOAT2 },
			{ std::type_index(typeid(XMFLOAT3)), MaterialParameterType::FLOAT3 },
			{ std::type_index(typeid(XMFLOAT4)), MaterialParameterType::FLOAT4 },
			{ std::type_index(typeid(XMINT2)), MaterialParameterType::INT2 },
			{ std::type_index(typeid(XMINT3)), MaterialParameterType::INT3 },
			{ std::type_index(typeid(XMINT4)), MaterialParameterType::INT4 },
			{ std::type_index(typeid(XMUINT2)), MaterialParameterType::UINT2 },
			{ std::type_index(typeid(XMUINT3)), MaterialParameterType::UINT3 },
			{ std::type_index(typeid(XMUINT4)), MaterialParameterType::UINT4 },
		};

		using MaterialParameterTypesPack = ParameterPack<bool, int32_t, uint32_t, float, UUID, XMFLOAT2, XMFLOAT3, XMFLOAT4, XMUINT2, XMUINT3, XMUINT4, XMINT2, XMINT3, XMINT4>;
	}
	
	enum class MaterialParameterPrecision
	{
		UNDEFINED,
		FLOAT,	// 32 bytes
		HALF	// 16 bytes
	};

	using MaterialParameterValue = material_utils::MaterialParameterTypesPack::Apply<std::variant>;
}