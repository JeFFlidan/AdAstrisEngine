#pragma once

#include "core/math_base.h"
#include "core/template_utils.h"
#include "engine_core/uuid.h"
#include <variant>

namespace ad_astris::ecore
{
	enum class MaterialParameterType
	{
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
		INT4,
		COUNT	// DON't USE
	};

	struct MaterialBool
	{
		uint32_t val{ 0 };
	};

	inline void to_json(nlohmann::json& j, const MaterialBool& value)
	{
		j = { { "val", value.val } };
	}

	inline void from_json(const nlohmann::json& j, MaterialBool& value)
	{
		j.at("val").get_to(value.val);
	}

	namespace internal
	{
		// If the parameter type is UUID, when copying the material data to the GPU, that UUID will be replaced by the descriptor index, so the type size of this member in the shader = 4 bytes, not 8
		inline constexpr uint32_t MATERIAL_PARAMETER_TYPE_SIZES[(int)MaterialParameterType::COUNT] = {
			sizeof(MaterialBool),
			sizeof(int32_t),
			sizeof(uint32_t),
			sizeof(float),
			sizeof(uint32_t), // UUID SIZE!!!
			sizeof(XMFLOAT2),
			sizeof(XMFLOAT3),
			sizeof(XMFLOAT4),
			sizeof(XMUINT2),
			sizeof(XMUINT3),
			sizeof(XMUINT4),
			sizeof(XMINT2),
			sizeof(XMINT3),
			sizeof(XMINT4),
		};

		using MaterialParameterTypesPack = ParameterPack<MaterialBool, int32_t, uint32_t, float, UUID, XMFLOAT2, XMFLOAT3, XMFLOAT4, XMUINT2, XMUINT3, XMUINT4, XMINT2, XMINT3, XMINT4>;
	}
	
	enum class MaterialParameterPrecision
	{
		UNDEFINED,
		FLOAT,	// 32 bytes
		HALF	// 16 bytes
	};

	using MaterialParameterValue = internal::MaterialParameterTypesPack::Apply<std::variant>;
	using MaterialParameterValueSerializer = internal::MaterialParameterTypesPack::Apply<VariantSerializer>;
}