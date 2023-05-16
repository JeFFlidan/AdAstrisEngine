#pragma once

#include "engine_core/uuid.h"
#include "file_system/file_system.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <string>

namespace ad_astris::ecore::model
{
	struct VertexF32
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec2 texCoord;
	};

	enum class VertexFormat
	{
		UNKNOWN = 0,
		F32,
	};

	struct ModelBounds
	{
		glm::vec3 origin;
		float radius;
		glm::vec3 extents;
	};
	
	struct StaticModelInfo
	{
		// Data for operations under hood
		UUID uuid;
		uint64_t vertexBufferSize{ 0 };
		uint64_t indexBufferSize{ 0 };
		ModelBounds bounds;
		VertexFormat vertexFormat{ VertexFormat::UNKNOWN };
		std::string originalFile;
		std::vector<std::string> materialsName;		// Materials name from model file
	};

	class Utils
	{
		public:
			static std::string get_str_vertex_format(VertexFormat format);
			static VertexFormat get_enum_vertex_format(std::string format);
			static std::string pack_static_model_info(StaticModelInfo* info);
			static StaticModelInfo unpack_static_model_info(std::string& strMetaData);
			static ModelBounds calculate_model_bounds(VertexF32* vertices, uint64_t count);
			static void calculate_tangent(VertexF32* vertices);
	};
}