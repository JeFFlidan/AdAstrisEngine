#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <stdint.h>
#include <vector>
#include <string>

namespace ad_astris::resource
{
	enum class CompressionMode
	{
		NONE = 0,
		LZ4,
	};

	enum class ResourceType
	{
		MODEL,
		TEXTURE,
		SCENE,
		MATERIAL,
	};
	
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

	enum class ModelType
	{
		STATIC,
		MOVABLE
	};

	// I will add much more properties in the future
	// modelData is stored in the heap so I should think about deleting it
	struct ModelInfo
	{
		// Data for operations under hood
		uint64_t vertexBufferSize{ 0 };
		uint64_t indexBufferSize{ 0 };
		ModelBounds bounds;
		uint8_t* modelData;
		VertexFormat vertexFormat{ VertexFormat::UNKNOWN };
		CompressionMode compressionMode{ CompressionMode::NONE };

		// Data to edit in engine
		std::string name;
		glm::vec3 translation;
		glm::vec4 rotation;
		glm::vec3 scale;
		ModelType type;
		bool isShadowCasted;
		std::vector<std::string> materialsName;

	};

	struct TextureInfo
	{
		
	};
}