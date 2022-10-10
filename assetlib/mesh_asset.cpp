#include <cstring>
#include <iostream>

#include <json.hpp>
#include <lz4.h>

#include <asset_loader.h>
#include <mesh_asset.h>

namespace assets
{
	VertexFormat parse_vertex_format(const char* format)
	{
		if (strcmp(format, "PNCV_F32"))
			return VertexFormat::PNCV_F32;
		else if (strcmp(format, "P32N8C8V16"))
			return VertexFormat::P32N8C8V16;
		else
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
}
