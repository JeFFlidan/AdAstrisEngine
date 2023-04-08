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

	ModelBounds calculate_model_bounds(VertexF32* vertices, uint64_t count);
	void calculate_tangent(VertexF32* vertices);

	void set_up_basic_model_info(ModelInfo* info);
}
