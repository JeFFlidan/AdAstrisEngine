#pragma once
#include <asset_loader.h>

namespace assets
{
	enum class TextureFormat : uint32_t
	{
		Unknown = 0,
		RGBA8 = 1,
		RGB8 = 2
	};

	struct PageInfoxs
	{
		uint32_t width;
		uint32_t height;
		uint32_t compressedSize;
		uint32_t originalSize;
	};

	struct TextureInfo
	{
		uint64_t textureSize;
		TextureFormat textureFormat;
		CompressionMode compressionMode;
		uint32_t pixelSize[3];
		std::string originalFile;
	};

	TextureInfo read_texture_info(AssetFile* file);
	void unpack_texture(TextureInfo* info, const char* sourceBuffer, size_t sourceSize, char* destination);
	AssetFile pack_texture(TextureInfo* info, void* pixelData);
}
