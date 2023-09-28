#pragma once

#include "engine_core/uuid.h"
#include <string>

namespace ad_astris::ecore::texture
{
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
	
	struct Texture2DInfo
	{
		// Data for operations under hood
		UUID uuid;
		uint64_t size{ 0 };
		uint64_t width{ 0 };
		uint64_t height{ 0 };
		std::string originalFile;

		// Data to edit in the engine
		MipmapMode mipmapMode;
		RuntimeCompressionMode runtimeCompressionMode;
		TilingMode tilingX;
		TilingMode tilingY;
		bool sRGB;
		uint32_t brightness;
		uint32_t saturation;
	};
	
	class Utils
	{
		public: 
			static std::string get_str_mipmap_mode(MipmapMode mode);
			static MipmapMode get_enum_mipmap_mode(std::string type);

			static std::string get_str_runtime_compression(RuntimeCompressionMode mode);
			static RuntimeCompressionMode get_enum_runtime_compression(std::string mode);

			static std::string get_str_tiling_mode(TilingMode mode);
			static TilingMode get_enum_tiling_mode(std::string mode);

			static std::string pack_texture2D_info(Texture2DInfo* info);
			static Texture2DInfo unpack_texture2D_info(std::string strMetaData);
	};
}