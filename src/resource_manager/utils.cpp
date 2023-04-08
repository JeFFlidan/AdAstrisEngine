#include "utils.h"
#include <cmath>
#include <limits>

using namespace ad_astris::resource;

std::string utils::get_resource_name(io::URI& uri)
{
	std::string path(uri.c_str());
	auto index1 = path.find_last_of("\\");
	if (index1 == std::string::npos)
	{
		index1 = path.find_last_of("/");
	}

	path.erase(path.begin(), path.begin() + index1 + 1);
	auto index2 = path.find(".");
	path.erase(path.begin() + index2, path.end());
	return path;
}

std::string utils::get_str_resource_type(ResourceType type)
{
	switch (type)
	{
		case ResourceType::MODEL:
			return "model";
		case ResourceType::SCENE:
			return "scene";
		case ResourceType::TEXTURE:
			return "texture";
		case ResourceType::MATERIAL:
			return "material";
	}
}

ResourceType utils::get_enum_resource_type(std::string& type)
{
	if (type == "model")
		return ResourceType::MODEL;
	if (type == "texture")
		return ResourceType::TEXTURE;
	if (type == "scene")
		return ResourceType::SCENE;
	if (type == "material")
		return ResourceType::MATERIAL;
}

std::string utils::get_str_vertex_format(VertexFormat format)
{
	switch (format)
	{
		case VertexFormat::F32:
			return "f32";
		case VertexFormat::UNKNOWN:
			return "unknown";
	}
}

VertexFormat utils::get_enum_vertex_format(std::string& format)
{
	if (format == "f32")
		return VertexFormat::F32;
	if (format == "unknown")
		return VertexFormat::UNKNOWN;
}

std::string utils::get_str_compression_mode(CompressionMode mode)
{
	switch (mode)
	{
		case CompressionMode::NONE:
			return "none";
		case CompressionMode::LZ4:
			return "lz4";
	}
}

CompressionMode utils::get_enum_compression_mode(std::string& mode)
{
	if (mode == "none")
		return CompressionMode::NONE;
	if (mode == "lz4")
		return CompressionMode::LZ4;
}

std::string utils::get_str_model_type(ModelType type)
{
	switch (type)
	{
		case ModelType::STATIC:
			return "static";
		case ModelType::MOVABLE:
			return "movable";
	}
}

ModelType utils::get_enum_model_type(std::string& type)
{
	if (type == "static")
		return ModelType::STATIC;
	if (type == "movable")
		return ModelType::MOVABLE;
}

ModelBounds utils::calculate_model_bounds(VertexF32* vertices, uint64_t count)
{
	ModelBounds bounds;
	
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

	bounds.extents.x = (max[0] - min[0]) / 2.0f;
	bounds.extents.y = (max[1] - min[1]) / 2.0f;
	bounds.extents.z = (max[2] - min[2]) / 2.0f;

	bounds.origin.x = bounds.extents.x + min[0];
	bounds.origin.y = bounds.extents.y + min[1];
	bounds.origin.z = bounds.extents.z + min[2];

	float r2 = 0;
	for (int i = 0; i != count; ++i)
	{
		float offset[3];
		offset[0] = vertices[i].position.x - bounds.origin.x;
		offset[1] = vertices[i].position.y - bounds.origin.y;
		offset[2] = vertices[i].position.z - bounds.origin.z;

		float distance = offset[0] * offset[0] + offset[1] * offset[1] + offset[2] * offset[2];
		r2 = std::max(r2, distance);
	}

	bounds.radius = std::sqrt(r2);

	return bounds;
}

void utils::calculate_tangent(VertexF32* vertices)
{
	VertexF32& firstVert = vertices[0];
	VertexF32& secondVert = vertices[1];
	VertexF32& thirdVert = vertices[2];
	float vertPos1[3] = { firstVert.position.x, firstVert.position.y, firstVert.position.z };
	float vertPos2[3] = { secondVert.position.x, secondVert.position.y, secondVert.position.z };
	float vertPos3[3] = { thirdVert.position.x, thirdVert.position.y, thirdVert.position.z };
	float vertUv1[2] = { firstVert.texCoord.x, firstVert.texCoord.y };
	float vertUv2[2] = { secondVert.texCoord.x, secondVert.texCoord.y };
	float vertUv3[2] = { thirdVert.texCoord.x, thirdVert.texCoord.y };
	float edge1[3] = { vertPos2[0] - vertPos1[0], vertPos2[1] - vertPos1[1], vertPos2[2] - vertPos1[2] };
	float edge2[3] = { vertPos3[0] - vertPos1[0], vertPos3[1] - vertPos1[1], vertPos3[2] - vertPos1[2] };
	float deltaUV1[2] = { vertUv2[0] - vertUv1[0], vertUv2[1] - vertUv1[1] };
	float deltaUV2[2] = { vertUv3[0] - vertUv1[0], vertUv3[1] - vertUv1[1] };
	float f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);
	float tangent[3];
	tangent[0] = f * (deltaUV2[1] * edge1[0] - deltaUV1[1] * edge2[0]);
	tangent[1] = f * (deltaUV2[1] * edge1[1] - deltaUV1[1] * edge2[1]);
	tangent[2] = f * (deltaUV2[1] * edge1[2] - deltaUV1[1] * edge2[2]);
	firstVert.tangent.x = tangent[0];
	firstVert.tangent.y = tangent[1];
	firstVert.tangent.z = tangent[2];
	secondVert.tangent.x = tangent[0];
	secondVert.tangent.y = tangent[1];
	secondVert.tangent.z = tangent[2];
	thirdVert.tangent.x = tangent[0];
	thirdVert.tangent.y = tangent[1];
	thirdVert.tangent.z = tangent[2];
}

void utils::set_up_basic_model_info(ModelInfo* info)
{
	info->vertexFormat = VertexFormat::F32;
	info->compressionMode = CompressionMode::LZ4;
	info->translation = glm::vec3(0.0f);
	info->rotation = glm::vec4(0.0f);
	info->scale = glm::vec3(1.0f);
	info->type = ModelType::STATIC;
	info->isShadowCasted = true;
}
