#include <cmath>
#include <cstring>
#include <iostream>

#include <json.hpp>
#include <limits>
#include <lz4.h>

#include <asset_loader.h>
#include <mesh_asset.h>

namespace assets
{
	VertexFormat parse_vertex_format(const char* format)
	{
		if (strcmp(format, "PNCV_F32"))
			return VertexFormat::PNCV_F32;
		if (strcmp(format, "P32N8C8V16"))
			return VertexFormat::P32N8C8V16;
		return VertexFormat::Unknown;
	}

	AssetFile pack_mesh(MeshInfo* info, char* vertexData, char* indexData)
	{
		nlohmann::json mesh_metadata;

		if (info->vertexFormat == VertexFormat::PNCV_F32)
			mesh_metadata["vertex_format"] = "PNCV_F32";
		else if (info->vertexFormat == VertexFormat::P32N8C8V16)
			mesh_metadata["vertex_format"] = "P32N8C8V16";

		mesh_metadata["vertex_buffer_size"] = info->vertexBufferSize;
		mesh_metadata["index_buffer_size"] = info->indexBufferSize;
		mesh_metadata["index_size"] = info->indexSize;
		mesh_metadata["original_file"] = info->originalFile;

		std::vector<float> boundsData;
		boundsData.resize(7);

		boundsData[0] = info->bounds.origin[0];
		boundsData[1] = info->bounds.origin[1];
		boundsData[2] = info->bounds.origin[2];

		boundsData[3] = info->bounds.radius;

		boundsData[4] = info->bounds.extents[0];
		boundsData[5] = info->bounds.extents[1];
		boundsData[6] = info->bounds.extents[2];

		mesh_metadata["bounds"] = boundsData;
		
		AssetFile file;
		file.type[0] = 'M';
		file.type[1] = 'E';
		file.type[2] = 'S';
		file.type[3] = 'H';
		file.version = '1';

		uint64_t vertBufSize = info->vertexBufferSize;
		uint64_t indBufSize = info->indexBufferSize;

		std::vector<char> tempBuffer(vertBufSize + indBufSize);
		memcpy(tempBuffer.data(), vertexData, vertBufSize);
		memcpy(tempBuffer.data() + vertBufSize, indexData, indBufSize);

		int compressStaging = LZ4_compressBound(vertBufSize + indBufSize);
		file.binaryBlob.resize(compressStaging);
		int compressedSize = LZ4_compress_default(
			(const char*)tempBuffer.data(), 
			file.binaryBlob.data(), 
			vertBufSize + indBufSize,
			compressStaging
			);
		file.binaryBlob.resize(compressedSize);

		mesh_metadata["compression"] = "LZ4";

		std::string meshInfo = mesh_metadata.dump();
		file.json = meshInfo;

		return file;
	}

	MeshInfo read_mesh_info(AssetFile* file)
	{
		MeshInfo info;
		nlohmann::json mesh_metadata = nlohmann::json::parse(file->json);

		std::string formatStr = mesh_metadata["vertex_format"];
		info.vertexFormat = parse_vertex_format(formatStr.c_str());

		std::string compressionStr = mesh_metadata["compression"];
		info.compressionMode = parse_compression(compressionStr.c_str());

		info.vertexBufferSize = mesh_metadata["vertex_buffer_size"];
		info.indexBufferSize = mesh_metadata["index_buffer_size"];
		info.originalFile = mesh_metadata["original_file"];
		info.indexSize = '1';

		std::vector<float> boundsData;
		boundsData.resize(7);

		boundsData = mesh_metadata["bounds"].get<std::vector<float>>();

		info.bounds.origin[0] = boundsData[0];
		info.bounds.origin[1] = boundsData[1];
		info.bounds.origin[2] = boundsData[2];

		info.bounds.radius = boundsData[3];
		
		info.bounds.extents[0] = boundsData[4];
		info.bounds.extents[1] = boundsData[5];
		info.bounds.extents[2] = boundsData[6];
		
		return info;
	}

	void unpack_mesh(MeshInfo* info, const char* sourceBuffer, size_t sourceSize, char* vertexBuffer, char* indexBuffer)
	{
		if (info->compressionMode == CompressionMode::LZ4)
		{
			uint64_t tempBufferSize = info->vertexBufferSize + info->indexBufferSize;
			std::vector<char> tempBuffer(tempBufferSize);
			LZ4_decompress_safe(sourceBuffer, tempBuffer.data(), sourceSize, tempBufferSize);
			
			memcpy(vertexBuffer, tempBuffer.data(), info->vertexBufferSize);
			memcpy(indexBuffer, tempBuffer.data() + info->vertexBufferSize, info->indexBufferSize);
		}
		else
		{
			memcpy(vertexBuffer, sourceBuffer, info->vertexBufferSize);
			memcpy(indexBuffer, sourceBuffer + info->vertexBufferSize, info->indexBufferSize);
		}
	}

    MeshBounds calculate_bounds(Vertex_f32_PNCV* vertices, size_t count)
	{
		// I have to read about bounds. Code was taker from https://github.com/vblanco20-1/vulkan-guide/blob/engine/assetlib/mesh_asset.cpp
		
		MeshBounds bounds;

		float min[3] = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
		float max[3] = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };

		for (int i = 0; i != count; ++i)
		{
			min[0] = std::min(min[0], vertices[i].position[0]);
			min[1] = std::min(min[1], vertices[i].position[1]);
			min[2] = std::min(min[2], vertices[i].position[2]);

			max[0] = std::max(max[0], vertices[i].position[0]);
			max[1] = std::max(max[1], vertices[i].position[1]);
			max[2] = std::max(max[2], vertices[i].position[2]);
		}

		bounds.extents[0] = (max[0] - min[0]) / 2.0f;
		bounds.extents[1] = (max[1] - min[1]) / 2.0f;
		bounds.extents[2] = (max[2] - min[2]) / 2.0f;

		bounds.origin[0] = bounds.extents[0] + min[0];
		bounds.origin[1] = bounds.extents[1] + min[1];
		bounds.origin[2] = bounds.extents[2] + min[2];

		float r2 = 0;
		for (int i = 0; i != count; ++i)
		{
			float offset[3];
			offset[0] = vertices[i].position[0] - bounds.origin[0];
			offset[1] = vertices[i].position[1] - bounds.origin[1];
			offset[2] = vertices[i].position[2] - bounds.origin[2];

			float distance = offset[0] * offset[0] + offset[1] * offset[1] + offset[2] * offset[2];
			r2 = std::max(r2, distance);
		}

		bounds.radius = std::sqrt(r2);
		
		return bounds;
	}
}
