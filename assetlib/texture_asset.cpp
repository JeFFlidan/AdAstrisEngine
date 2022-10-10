#include <json.hpp>
#include <lz4.h>
#include <iostream>
#include <texture_asset.h>
#include <cstring>

namespace assets
{
	TextureFormat parse_format(const char* f)
	{
		if (strcmp(f, "RGBA8") == 0)
			return TextureFormat::RGBA8;
		else if (strcmp(f, "RGB8") == 0)
			return TextureFormat::RGB8;
		else
			return TextureFormat::Unknown;
	}

	std::string parse_format(TextureFormat format)
	{
		switch (format)
		{
			case (assets::TextureFormat::RGBA8):
				return "RGBA8";
			case (assets::TextureFormat::RGB8):
				return "RGB8";
			case (assets::TextureFormat::Unknown):
				return "Unknown";
		}
	}

	AssetFile pack_texture(TextureInfo* info, void* pixelData)
	{
		nlohmann::json texture_metadata;
		texture_metadata["format"] = parse_format(info->textureFormat);
		texture_metadata["width"] = info->pixelSize[0];
		texture_metadata["height"] = info->pixelSize[1];
		texture_metadata["buffer_size"] = info->textureSize;
		texture_metadata["original_file"] = info->originalFile;

		AssetFile file;
		file.type[0] = 'T';
		file.type[1] = 'E';
		file.type[2] = 'X';
		file.type[3] = 'I';
		file.version = 1;

		int compressStaging = LZ4_compressBound(info->textureSize);
		file.binaryBlob.resize(compressStaging);
		int compressedSize = LZ4_compress_default((const char*)pixelData, file.binaryBlob.data(), info->textureSize, compressStaging);
		file.binaryBlob.resize(compressedSize);

		texture_metadata["compression"] = "LZ4";

		std::string stringfied = texture_metadata.dump();
		file.json = stringfied;

		return file;
	}

	TextureInfo read_texture_info(AssetFile* file)
	{
		TextureInfo info;
		nlohmann::json texture_metadata = nlohmann::json::parse(file->json);

		std::string formatString = texture_metadata["format"];
		info.textureFormat = parse_format(formatString.c_str());

		std::string compressionString = texture_metadata["compression"];
		info.compressionMode = parse_compression(compressionString.c_str());

		info.pixelSize[0] = texture_metadata["width"];
		info.pixelSize[1] = texture_metadata["height"];
		info.textureSize = texture_metadata["buffer_size"];
		info.originalFile = texture_metadata["original_file"];

		return info;
	}

	void unpack_texture(TextureInfo* info, const char* sourceBuffer, size_t sourceSize, char* destination)
	{
		if (info->compressionMode == CompressionMode::LZ4)
		{
			LZ4_decompress_safe(sourceBuffer, destination, sourceSize, info->textureSize);
		}
		else
		{
			memcpy(destination, sourceBuffer, sourceSize);
		}
	}
}
