#pragma once

#include "engine_core/uuid.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <stdint.h>
#include <vector>
#include <string>
#include <map>

namespace ad_astris::resource
{
	template<typename T>
	struct ResourceMethods
	{
		
	};
	
	enum class ResourceType
	{
		UNDEFINED,
		MODEL,
		TEXTURE,
		LEVEL,
		MATERIAL,
	};
	
	struct ResourceInfo
	{
		UUID uuid;
		ResourceType type{ ResourceType::UNDEFINED };
		std::string metaData;
		uint64_t dataSize{ 0 };
		uint8_t* data{ nullptr };
	};
	
	enum class CompressionMode
	{
		NONE = 0,
		LZ4,
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
		UUID uuid;
		uint64_t vertexBufferSize{ 0 };
		uint64_t indexBufferSize{ 0 };
		ModelBounds bounds;
		VertexFormat vertexFormat{ VertexFormat::UNKNOWN };
		CompressionMode compressionMode{ CompressionMode::NONE };
		std::string originalFile;
		std::vector<std::string> materialsName;		// Materials name from model file
		std::string name;
	};

	// Instance in engine. Every object in the scene has its own info
	struct ModelInstanceInfo
	{
		UUID parentUUID;
		std::string name;
		glm::vec3 translation;
		glm::vec3 rotationAxis;
		float rotationAngle;
		glm::vec3 scale;
		ModelType type;
		bool isShadowCasted;
		std::vector<UUID> engineMaterials;
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
		UUID uuid;
		uint64_t size{ 0 };
		uint64_t width{0};
		uint64_t height{ 0 };
		CompressionMode compressionMode{ CompressionMode::NONE };
		std::string originalFile;

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

	struct LevelEngineInfo
	{
		UUID uuid;
		std::map<UUID, std::vector<ModelInstanceInfo>> modelInstancesInfo;
		std::vector<UUID> texturesUUID;
	};

	struct LevelFileInfo
	{
		UUID uuid;
		std::vector<std::string> modelInstancesInfo;
		std::vector<std::string> texturesInfo;
	};
}