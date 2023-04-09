#pragma once

#include "resource_formats.h"
#include <file_system/file_system.h>

namespace ad_astris::resource::utils
{
	std::string get_resource_name(io::URI& uri);
	
	std::string get_str_resource_type(ResourceType type);
	ResourceType get_enum_resource_type(std::string& type);
	
	std::string get_str_vertex_format(VertexFormat format);
	VertexFormat get_enum_vertex_format(std::string& format);
	
	std::string get_str_compression_mode(CompressionMode mode);
	CompressionMode get_enum_compression_mode(std::string& mode);

	std::string get_str_model_type(ModelType type);
	ModelType get_enum_model_type(std::string& type);

	std::string get_str_mipmap_mode(MipmapMode mode);
	MipmapMode get_enum_mipmap_mode(std::string& type);

	std::string get_str_runtime_compression(RuntimeCompressionMode mode);
	RuntimeCompressionMode get_enum_runtime_compression(std::string& mode);

	std::string get_str_tiling_mode(TilingMode mode);
	TilingMode get_enum_tiling_mode(std::string& mode);

	ModelBounds calculate_model_bounds(VertexF32* vertices, uint64_t count);
	void calculate_tangent(VertexF32* vertices);

	void set_up_basic_model_info(ModelInfo* info);
	void set_up_basic_texture_info(TextureInfo* info);

	ResourceInfo pack_model_info(ModelInfo* modelInfo);
	ModelInfo* unpack_model_info(ResourceInfo* resourceInfo);

	ResourceInfo pack_texture_info(TextureInfo* textureInfo);
	TextureInfo* unpack_texture_info(ResourceInfo* resourceInfo);
}
