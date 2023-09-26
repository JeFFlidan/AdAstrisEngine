#pragma once

#include "engine_core/uuid.h"
#include "math_base.h"

#include <json/json.hpp>
#include <glm/glm/glm.hpp>

#include <glm/glm/detail/type_quat.hpp>

namespace glm
{
	inline void to_json(nlohmann::json& j, const vec2& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y } };
	}

	inline void from_json(const nlohmann::json& j, vec2& vec)
	{
		vec.x = j.at("x").get<float>();
		vec.y = j.at("y").get<float>();
	}

	inline void to_json(nlohmann::json& j, const ivec2& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y } };
	}

	inline void from_json(const nlohmann::json& j, ivec2& vec)
	{
		vec.x = j.at("x").get<int32_t>();
		vec.y = j.at("y").get<int32_t>();
	}

	inline void to_json(nlohmann::json& j, const uvec2& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y } };
	}

	inline void from_json(const nlohmann::json& j, uvec2& vec)
	{
		vec.x = j.at("x").get<uint32_t>();
		vec.y = j.at("y").get<uint32_t>();
	}
	
	inline void to_json(nlohmann::json& j, const vec3& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y }, { "z", vec.z } };
	}

	inline void from_json(const nlohmann::json& j, vec3& vec)
	{
		vec.x = j.at("x").get<float>();
		vec.y = j.at("y").get<float>();
		vec.z = j.at("z").get<float>();
	}

	inline void to_json(nlohmann::json& j, const ivec3& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y }, { "z", vec.z } };
	}

	inline void from_json(const nlohmann::json& j, ivec3& vec)
	{
		vec.x = j.at("x").get<int32_t>();
		vec.y = j.at("y").get<int32_t>();
		vec.z = j.at("z").get<int32_t>();
	}

	inline void to_json(nlohmann::json& j, const uvec3& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y }, { "z", vec.z } };
	}

	inline void from_json(const nlohmann::json& j, uvec3& vec)
	{
		vec.x = j.at("x").get<uint32_t>();
		vec.y = j.at("y").get<uint32_t>();
		vec.z = j.at("z").get<uint32_t>();
	}

	inline void to_json(nlohmann::json& j, const glm::vec4& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y }, { "z", vec.z }, {"w", vec.w} };
	}

	inline void from_json(const nlohmann::json& j, vec4& vec)
	{
		vec.x = j.at("x").get<float>();
		vec.y = j.at("y").get<float>();
		vec.z = j.at("z").get<float>();
		vec.w = j.at("w").get<float>();
	}

	inline void to_json(nlohmann::json& j, const ivec4& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y }, { "z", vec.z }, {"w", vec.w} };
	}

	inline void from_json(const nlohmann::json& j, ivec4& vec)
	{
		vec.x = j.at("x").get<int32_t>();
		vec.y = j.at("y").get<int32_t>();
		vec.z = j.at("z").get<int32_t>();
		vec.w = j.at("w").get<int32_t>();
	}

	inline void to_json(nlohmann::json& j, const uvec4& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y }, { "z", vec.z }, {"w", vec.w} };
	}

	inline void from_json(const nlohmann::json& j, uvec4& vec)
	{
		vec.x = j.at("x").get<uint32_t>();
		vec.y = j.at("y").get<uint32_t>();
		vec.z = j.at("z").get<uint32_t>();
		vec.w = j.at("w").get<uint32_t>();
	}

	inline void to_json(nlohmann::json& j, const mat2x2& mat)
	{
		j = {
			{ "[0][0]", mat[0][0] },
			{ "[0][1]", mat[0][1] },
			{ "[1][0]", mat[1][0] },
			{ "[1][1]", mat[1][1] }
		};
	}


	inline void to_json(nlohmann::json& j, const quat& quat)
	{
		j = { { "x", quat.x }, { "y", quat.y }, { "z", quat.z }, {"w", quat.w} };
	}

	inline void from_json(const nlohmann::json& j, quat& quat)
	{
		quat.x = j.at("x").get<float>();
		quat.y = j.at("y").get<float>();
		quat.z = j.at("z").get<float>();
		quat.w = j.at("w").get<float>();
	}


}

namespace ad_astris
{
	inline void to_json(nlohmann::json& j, const UUID& uuid)
	{
		j = (uint64_t)uuid;
	}

	inline void from_json(const nlohmann::json& j, UUID& uuid)
	{
		uuid = UUID(j.get<uint64_t>());
	}
}

namespace DirectX
{
	inline void to_json(nlohmann::json& j, const XMFLOAT2& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y } };
	}

	inline void from_json(const nlohmann::json& j, XMFLOAT2& vec)
	{
		vec.x = j.at("x").get<float>();
		vec.y = j.at("y").get<float>();
	}

	inline void to_json(nlohmann::json& j, const XMINT2& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y } };
	}

	inline void from_json(const nlohmann::json& j, XMINT2& vec)
	{
		vec.x = j.at("x").get<int32_t>();
		vec.y = j.at("y").get<int32_t>();
	}

	inline void to_json(nlohmann::json& j, const XMUINT2& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y } };
	}

	inline void from_json(const nlohmann::json& j, XMUINT2& vec)
	{
		vec.x = j.at("x").get<uint32_t>();
		vec.y = j.at("y").get<uint32_t>();
	}

	inline void to_json(nlohmann::json& j, const XMFLOAT3& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y }, { "z", vec.z } };
	}

	inline void from_json(const nlohmann::json& j, XMFLOAT3& vec)
	{
		vec.x = j.at("x").get<float>();
		vec.y = j.at("y").get<float>();
		vec.z = j.at("z").get<float>();
	}

	inline void to_json(nlohmann::json& j, const XMINT3& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y }, { "z", vec.z } };
	}

	inline void from_json(const nlohmann::json& j, XMINT3& vec)
	{
		vec.x = j.at("x").get<int32_t>();
		vec.y = j.at("y").get<int32_t>();
		vec.z = j.at("z").get<int32_t>();
	}

	inline void to_json(nlohmann::json& j, const XMUINT3& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y }, { "z", vec.z } };
	}

	inline void from_json(const nlohmann::json& j, XMUINT3& vec)
	{
		vec.x = j.at("x").get<uint32_t>();
		vec.y = j.at("y").get<uint32_t>();
		vec.z = j.at("z").get<uint32_t>();
	}

	inline void to_json(nlohmann::json& j, const XMFLOAT4& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y }, { "z", vec.z }, {"w", vec.w} };
	}

	inline void from_json(const nlohmann::json& j, XMFLOAT4& vec)
	{
		vec.x = j.at("x").get<float>();
		vec.y = j.at("y").get<float>();
		vec.z = j.at("z").get<float>();
		vec.w = j.at("w").get<float>();
	}

	inline void to_json(nlohmann::json& j, const XMINT4& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y }, { "z", vec.z }, {"w", vec.w} };
	}

	inline void from_json(const nlohmann::json& j, XMINT4& vec)
	{
		vec.x = j.at("x").get<int32_t>();
		vec.y = j.at("y").get<int32_t>();
		vec.z = j.at("z").get<int32_t>();
		vec.w = j.at("w").get<int32_t>();
	}

	inline void to_json(nlohmann::json& j, const XMUINT4& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y }, { "z", vec.z }, {"w", vec.w} };
	}

	inline void from_json(const nlohmann::json& j, XMUINT4& vec)
	{
		vec.x = j.at("x").get<uint32_t>();
		vec.y = j.at("y").get<uint32_t>();
		vec.z = j.at("z").get<uint32_t>();
		vec.w = j.at("w").get<uint32_t>();
	}

	inline void to_json(nlohmann::json& j, const XMFLOAT3X3& mat)
	{
		j = {
			{ "[0][0]", mat(0, 0)},
			{ "[0][1]", mat(0, 1) },
			{ "[0][2]", mat(0, 2) },
			{ "[1][0]", mat(1, 0) },
			{ "[1][1]", mat(1, 1) },
			{ "[1][2]", mat(1, 2) },
			{ "[2][0]", mat(2, 0) },
			{ "[2][1]", mat(2, 1) },
			{ "[2][2]", mat(2, 2) }
		};
	}

	inline void from_json(const nlohmann::json& j, XMFLOAT3X3& mat)
	{
		mat(0, 0) = j.at("[0][0]").get<float>();
		mat(0, 1) = j.at("[0][1]").get<float>();
		mat(0, 2) = j.at("[0][2]").get<float>();
		mat(1, 0) = j.at("[1][0]").get<float>();
		mat(1, 1) = j.at("[1][1]").get<float>();
		mat(1, 2) = j.at("[1][2]").get<float>();
		mat(2, 0) = j.at("[2][0]").get<float>();
		mat(2, 1) = j.at("[2][1]").get<float>();
		mat(2, 2) = j.at("[2][2]").get<float>();
	}

	inline void to_json(nlohmann::json& j, const XMFLOAT3X4& mat)
	{
		j = {
			{ "[0][0]", mat(0, 0) },
			{ "[0][1]", mat(0, 1) },
			{ "[0][2]", mat(0, 2) },
			{ "[0][3]", mat(0, 3) },
			{ "[1][0]", mat(1, 0) },
			{ "[1][1]", mat(1, 1) },
			{ "[1][2]", mat(1, 2) },
			{ "[1][3]", mat(1, 3) },
			{ "[2][0]", mat(2, 0) },
			{ "[2][1]", mat(2, 1) },
			{ "[2][2]", mat(2, 2) },
			{ "[2][3]", mat(2, 3) }
		};
	}

	inline void from_json(const nlohmann::json& j, XMFLOAT3X4& mat)
	{
		mat(0, 0) = j.at("[0][0]").get<float>();
		mat(0, 1) = j.at("[0][1]").get<float>();
		mat(0, 2) = j.at("[0][2]").get<float>();
		mat(0, 3) = j.at("[0][3]").get<float>();
		mat(1, 0) = j.at("[1][0]").get<float>();
		mat(1, 1) = j.at("[1][1]").get<float>();
		mat(1, 2) = j.at("[1][2]").get<float>();
		mat(1, 3) = j.at("[1][3]").get<float>();
		mat(2, 0) = j.at("[2][0]").get<float>();
		mat(2, 1) = j.at("[2][1]").get<float>();
		mat(2, 2) = j.at("[2][2]").get<float>();
		mat(2, 3) = j.at("[2][3]").get<float>();
	}

	inline void to_json(nlohmann::json& j, const XMFLOAT4X4& mat)
	{
		j = {
			{ "[0][0]", mat(0, 0) },
			{ "[0][1]", mat(0, 1) },
			{ "[0][2]", mat(0, 2) },
			{ "[0][3]", mat(0, 3) },
			{ "[1][0]", mat(1, 0) },
			{ "[1][1]", mat(1, 1) },
			{ "[1][2]", mat(1, 2) },
			{ "[1][3]", mat(1, 3) },
			{ "[2][0]", mat(2, 0) },
			{ "[2][1]", mat(2, 1) },
			{ "[2][2]", mat(2, 2) },
			{ "[2][3]", mat(2, 3) },
			{ "[3][0]", mat(3, 0) },
			{ "[3][1]", mat(3, 1) },
			{ "[3][2]", mat(3, 2) },
			{ "[3][3]", mat(3, 3) }
		};
	}

	inline void from_json(const nlohmann::json& j, XMFLOAT4X4& mat)
	{
		mat(0, 0) = j.at("[0][0]").get<float>();
		mat(0, 1) = j.at("[0][1]").get<float>();
		mat(0, 2) = j.at("[0][2]").get<float>();
		mat(0, 3) = j.at("[0][3]").get<float>();
		mat(1, 0) = j.at("[1][0]").get<float>();
		mat(1, 1) = j.at("[1][1]").get<float>();
		mat(1, 2) = j.at("[1][2]").get<float>();
		mat(1, 3) = j.at("[1][3]").get<float>();
		mat(2, 0) = j.at("[2][0]").get<float>();
		mat(2, 1) = j.at("[2][1]").get<float>();
		mat(2, 2) = j.at("[2][2]").get<float>();
		mat(2, 3) = j.at("[2][3]").get<float>();
		mat(3, 0) = j.at("[3][0]").get<float>();
		mat(3, 1) = j.at("[3][1]").get<float>();
		mat(3, 2) = j.at("[3][2]").get<float>();
		mat(3, 3) = j.at("[3][3]").get<float>();
	}

	inline void to_json(nlohmann::json& j, const XMFLOAT4X3& mat)
	{
		j = {
			{ "[0][0]", mat(0, 0) },
			{ "[0][1]", mat(0, 1) },
			{ "[0][2]", mat(0, 2) },
			{ "[1][0]", mat(1, 0) },
			{ "[1][1]", mat(1, 1) },
			{ "[1][2]", mat(1, 2) },
			{ "[2][0]", mat(2, 0) },
			{ "[2][1]", mat(2, 1) },
			{ "[2][2]", mat(2, 2) },
			{ "[3][0]", mat(3, 0) },
			{ "[3][1]", mat(3, 1) },
			{ "[3][2]", mat(3, 2) }
		};
	}

	inline void from_json(const nlohmann::json& j, XMFLOAT4X3& mat)
	{
		mat(0, 0) = j.at("[0][0]").get<float>();
		mat(0, 1) = j.at("[0][1]").get<float>();
		mat(0, 2) = j.at("[0][2]").get<float>();
		mat(1, 0) = j.at("[1][0]").get<float>();
		mat(1, 1) = j.at("[1][1]").get<float>();
		mat(1, 2) = j.at("[1][2]").get<float>();
		mat(2, 0) = j.at("[2][0]").get<float>();
		mat(2, 1) = j.at("[2][1]").get<float>();
		mat(2, 2) = j.at("[2][2]").get<float>();
		mat(3, 0) = j.at("[3][0]").get<float>();
		mat(3, 1) = j.at("[3][1]").get<float>();
		mat(3, 2) = j.at("[3][2]").get<float>();
	}
}
