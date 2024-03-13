#include "texture.h"
#include "rhi/utils.h"
#include "core/constants.h"
#include "core/custom_objects_to_json.h"
#include <json/json.hpp>

using namespace ad_astris;
using namespace ecore;

constexpr const char* UUID_KEY = "uuid";
constexpr const char* SIZE_KEY = "size";
constexpr const char* WIDTH_KEY = "width";
constexpr const char* HEIGHT_KEY = "height";
constexpr const char* DEPTH_KEY = "depth";
constexpr const char* MIPMAP_MODE_KEY = "mipmap_mode";
constexpr const char* COMPRESSION_MODE_KEY = "compression_mode";
constexpr const char* TILING_X_KEY = "tiling_x";
constexpr const char* TILING_Y_KEY = "tiling_y";
constexpr const char* FORMAT_KEY = "format";
constexpr const char* COMPONENT_MAPPING_KEY = "component_mapping";
constexpr const char* BRIGHTNESS_KEY = "brightness";
constexpr const char* SATURATION_KEY = "saturation";
constexpr const char* IS_16_BIT_KEY = "is_16_bit";

Texture::Texture(const TextureInfo& textureInfo) : _textureInfo(textureInfo)
{
	
}

Texture::~Texture()
{
	delete[] _textureInfo.data;
}

void Texture::serialize(io::File* file)
{
	if (!_textureInfo.data)
	{
		LOG_ERROR("Texture::serialize(): Failed to serialize texture because data blob is invalid")
		return;
	}
	uint8_t* blob = new uint8_t[_textureInfo.size];
	memcpy(blob, _textureInfo.data, _textureInfo.size);

	nlohmann::json metadata;
	metadata[UUID_KEY] = _uuid;
	metadata[SIZE_KEY] = _textureInfo.size;
	metadata[WIDTH_KEY] = _textureInfo.width;
	metadata[HEIGHT_KEY] = _textureInfo.height;
	metadata[DEPTH_KEY] = _textureInfo.depth;
	metadata[MIPMAP_MODE_KEY] = texture::Utils::get_str_mipmap_mode(_textureInfo.mipmapMode);
	metadata[COMPRESSION_MODE_KEY] = texture::Utils::get_str_runtime_compression(_textureInfo.runtimeCompressionMode);
	metadata[TILING_X_KEY] = rhi::Utils::get_address_mode_str(_textureInfo.tilingX);
	metadata[TILING_Y_KEY] = rhi::Utils::get_address_mode_str(_textureInfo.tilingY);
	metadata[FORMAT_KEY] = rhi::Utils::get_format_str(_textureInfo.format);
	metadata[COMPONENT_MAPPING_KEY] = rhi::Utils::get_component_mapping_str(_textureInfo.mapping);
	metadata[BRIGHTNESS_KEY] = _textureInfo.brightness;
	metadata[SATURATION_KEY] = _textureInfo.saturation;
	metadata[IS_16_BIT_KEY] = _textureInfo.is16Bit;

	file->set_binary_blob(blob, _textureInfo.size);
	file->set_metadata(metadata.dump(JSON_INDENT));
}

void Texture::deserialize(io::File* file, ObjectName* objectName)
{
	_name = objectName;

	nlohmann::json metadata = nlohmann::json::parse(file->get_metadata());
	_uuid = metadata[UUID_KEY];
	_textureInfo.size = metadata[SIZE_KEY];
	_textureInfo.width = metadata[WIDTH_KEY];
	_textureInfo.height = metadata[HEIGHT_KEY];
	_textureInfo.depth = metadata[DEPTH_KEY];
	_textureInfo.mipmapMode = texture::Utils::get_enum_mipmap_mode(metadata[MIPMAP_MODE_KEY]);
	_textureInfo.runtimeCompressionMode = texture::Utils::get_enum_runtime_compression(metadata[COMPRESSION_MODE_KEY]);
	_textureInfo.tilingX = rhi::Utils::get_address_mode_enum(metadata[TILING_X_KEY]);
	_textureInfo.tilingY = rhi::Utils::get_address_mode_enum(metadata[TILING_Y_KEY]);
	_textureInfo.format = rhi::Utils::get_format_enum(metadata[FORMAT_KEY]);
	_textureInfo.mapping = rhi::Utils::get_component_mapping(metadata[COMPONENT_MAPPING_KEY]);
	_textureInfo.brightness = metadata[BRIGHTNESS_KEY];
	_textureInfo.saturation = metadata[SATURATION_KEY];
	_textureInfo.is16Bit = metadata[IS_16_BIT_KEY];
	
	_textureInfo.data = new uint8_t[file->get_binary_blob_size()];
	memcpy(_textureInfo.data, file->get_binary_blob(), file->get_binary_blob_size());
}

void Texture::accept(resource::IResourceVisitor& resourceVisitor)
{
	// TODO
}

void Texture::update_texture(uint8_t* textureData, uint64_t sizeInBytes)
{
	delete[] _textureInfo.data;

	_textureInfo.data = new uint8_t[sizeInBytes];
	_textureInfo.size = sizeInBytes;
	memcpy(_textureInfo.data, textureData, sizeInBytes);
}

void Texture::destroy_texture_data()
{
	delete[] _textureInfo.data;
	_textureInfo.data = nullptr;
}
