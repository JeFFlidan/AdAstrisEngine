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
		uint8_t* modelData{ nullptr };
		VertexFormat vertexFormat{ VertexFormat::UNKNOWN };
		CompressionMode compressionMode{ CompressionMode::NONE };

		// Data to edit in the engine
		std::string name;
		glm::vec3 translation;
		glm::vec4 rotation;
		glm::vec3 scale;
		ModelType type;
		bool isShadowCasted;
		std::vector<std::string> materialsName;

	};

	enum class MipmapMode
	{
		BASE_MIPMAPPING,
		NO_MIPMAPS
	};

	// I have to implement it in the engine
	enum class RuntimeCompressionMode
	{
		DXT1,
		DXT5
	};

	enum class TilingMode
	{
		REPEAT,
		CLAMP,
		MIRROR
	};

	struct TextureInfo
	{
		// Data for operations under hood
		uint64_t size{ 0 };
		uint64_t width{0};
		uint64_t height{ 0 };
		uint8_t* data{ nullptr };
		CompressionMode compressionMode{ CompressionMode::NONE };

		// Data to edit in the engine
		std::string name;
		MipmapMode mipmapMode;
		RuntimeCompressionMode runtimeCompressionMode;
		TilingMode tilingX;
		TilingMode tilingY;
		bool sRGB;
		uint32_t brightness;
		uint32_t saturation;
	};
}