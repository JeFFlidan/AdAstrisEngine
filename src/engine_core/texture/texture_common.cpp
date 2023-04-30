#include "texture_common.h"
#include <json.hpp>

using namespace ad_astris::ecore;

std::string texture::Utils::get_str_mipmap_mode(MipmapMode mode)
{
	switch (mode)
	{
		case MipmapMode::BASE_MIPMAPPING:
			return "base_mipmapping";
		case MipmapMode::NO_MIPMAPS:
			return "no_mipmaps";
	}
}

texture::MipmapMode texture::Utils::get_enum_mipmap_mode(std::string type)
{
	if (type == "base_mipmapping")
		return MipmapMode::BASE_MIPMAPPING;
	if (type == "no_mipmaps")
		return MipmapMode::NO_MIPMAPS;
}

std::string texture::Utils::get_str_runtime_compression(RuntimeCompressionMode mode)
{
	switch (mode)
	{
		case RuntimeCompressionMode::DXT1:
			return "dxt1";
		case RuntimeCompressionMode::DXT5:
			return "dxt5";
	}
}

texture::RuntimeCompressionMode texture::Utils::get_enum_runtime_compression(std::string mode)
{
	if (mode == "dxt1")
		return RuntimeCompressionMode::DXT1;
	if (mode == "dxt5")
		return RuntimeCompressionMode::DXT5;
}

std::string texture::Utils::get_str_tiling_mode(TilingMode mode)
{
	switch (mode)
	{
		case TilingMode::REPEAT:
			return "repeat";
		case TilingMode::CLAMP:
			return "clamp";
		case TilingMode::MIRROR:
			return "mirror";
	}
}

texture::TilingMode texture::Utils::get_enum_tiling_mode(std::string mode)
{
	if (mode == "repeat")
		return TilingMode::REPEAT;
	if (mode == "clamp")
		return TilingMode::CLAMP;
	if (mode == "mirror")
		return TilingMode::MIRROR;
}

std::string texture::Utils::pack_texture2D_info(Texture2DInfo* info)
{
	nlohmann::json textureMetaData;
	textureMetaData["original_file"] = info->originalFile;
	textureMetaData["texture_size"] = info->size;
	textureMetaData["texture_width"] = info->width;
	textureMetaData["texture_height"] = info->height;
	textureMetaData["mipmap_mode"] = get_str_mipmap_mode(info->mipmapMode);
	textureMetaData["runtime_compression"] = get_str_runtime_compression(info->runtimeCompressionMode);
	textureMetaData["tiling_x"] = get_str_tiling_mode(info->tilingX);
	textureMetaData["tiling_y"] = get_str_tiling_mode(info->tilingY);
	textureMetaData["sRGB"] = info->sRGB;
	textureMetaData["brightness"] = info->brightness;
	textureMetaData["saturation"] = info->saturation;
	return textureMetaData.dump();
}

texture::Texture2DInfo texture::Utils::unpack_texture2D_info(std::string strMetaData)
{
	Texture2DInfo info;
	
	nlohmann::json textureMetaData = nlohmann::json::parse(strMetaData);
	info.originalFile = textureMetaData["original_file"];
	info.size = textureMetaData["texture_size"];
	info.width = textureMetaData["texture_width"];
	info.height = textureMetaData["texture_height"];
	info.mipmapMode = get_enum_mipmap_mode(textureMetaData["mipmap_mode"]);
	info.runtimeCompressionMode = get_enum_runtime_compression(textureMetaData["runtime_compression"]);
	info.tilingX = get_enum_tiling_mode(textureMetaData["tiling_x"]);
	info.tilingY = get_enum_tiling_mode(textureMetaData["tiling_y"]);
	info.sRGB = textureMetaData["sRGB"];
	info.brightness = textureMetaData["brightness"];
	info.saturation = textureMetaData["saturation"];

	return info;
}
