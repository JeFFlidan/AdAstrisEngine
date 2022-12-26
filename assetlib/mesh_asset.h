#pragma once

#include <asset_loader.h>

namespace assets
{
	struct Vertex_f32_PNCV
	{
		float position[3];
		float normal[3];
		float tangent[3];
		float uv[2];
	};
	
	struct Vertex_P32N8C8V16
	{
		float position[3];
		uint8_t normal[3];
		uint8_t color[3];
		float uv[2];
	};

	enum class VertexFormat : uint32_t
	{
		Unknown = 0,
		PNCV_F32,
		P32N8C8V16
	};

	struct MeshBounds
	{
		float origin[3];
		float radius;
		float extents[3];
	};
	
	struct MeshInfo
	{
		uint64_t vertexBufferSize;
		uint64_t indexBufferSize;
		VertexFormat vertexFormat;
		MeshBounds bounds;
		char indexSize;
		CompressionMode compressionMode;
		std::string originalFile;
	};

	AssetFile pack_mesh(MeshInfo* info, char* vertexData, char* indexData);
	
	void unpack_mesh(MeshInfo* info, const char* sourceBuffer, size_t sourceSize, char* vertexBuffer, char* indexBuffer);
	
	MeshInfo read_mesh_info(AssetFile* file);

	MeshBounds calculate_bounds(Vertex_f32_PNCV* vertices, size_t count);
}
