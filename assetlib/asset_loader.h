#pragma once

#include <vector>
#include <string>

namespace assets
{
	struct AssetFile
	{
		char type[4];
		int version;
		std::string json;
		std::vector<char> binaryBlob;
	};

	enum class CompressionMode : uint32_t
	{
		None,
		LZ4
	};

	bool save_binaryFile(const char* path, const AssetFile& file);

	bool load_binaryFile(const char* path, AssetFile& outputFile);

	bool saveJson(const char* path, AssetFile& file);
	
	CompressionMode parse_compression(const char* f);
}