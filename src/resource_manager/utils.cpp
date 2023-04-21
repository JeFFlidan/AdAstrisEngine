#include "utils.h"
#include <json.hpp>
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
		case ResourceType::LEVEL:
			return "scene";
		case ResourceType::TEXTURE:
			return "texture";
		case ResourceType::MATERIAL:
			return "material";
	}
}

ResourceType utils::get_enum_resource_type(std::string type)
{
	if (type == "model")
		return ResourceType::MODEL;
	if (type == "texture")
		return ResourceType::TEXTURE;
	if (type == "level")
		return ResourceType::LEVEL;
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

VertexFormat utils::get_enum_vertex_format(std::string format)
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

CompressionMode utils::get_enum_compression_mode(std::string mode)
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

ModelType utils::get_enum_model_type(std::string type)
{
	if (type == "static")
		return ModelType::STATIC;
	if (type == "movable")
		return ModelType::MOVABLE;
}

std::string utils::get_str_mipmap_mode(MipmapMode mode)
{
	switch (mode)
	{
		case MipmapMode::BASE_MIPMAPPING:
			return "base_mipmapping";
		case MipmapMode::NO_MIPMAPS:
			return "no_mipmaps";
	}
}

MipmapMode utils::get_enum_mipmap_mode(std::string type)
{
	if (type == "base_mipmapping")
		return MipmapMode::BASE_MIPMAPPING;
	if (type == "no_mipmaps")
		return MipmapMode::NO_MIPMAPS;
}

std::string utils::get_str_runtime_compression(RuntimeCompressionMode mode)
{
	switch (mode)
	{
		case RuntimeCompressionMode::DXT1:
			return "dxt1";
		case RuntimeCompressionMode::DXT5:
			return "dxt5";
	}
}

RuntimeCompressionMode utils::get_enum_runtime_compression(std::string mode)
{
	if (mode == "dxt1")
		return RuntimeCompressionMode::DXT1;
	if (mode == "dxt5")
		return RuntimeCompressionMode::DXT5;
}

std::string utils::get_str_tiling_mode(TilingMode mode)
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
TilingMode utils::get_enum_tiling_mode(std::string mode)
{
	if (mode == "repeat")
		return TilingMode::REPEAT;
	if (mode == "clamp")
		return TilingMode::CLAMP;
	if (mode == "mirror")
		return TilingMode::MIRROR;
}

ModelBounds utils::calculate_model_bounds(VertexF32* vertices, uint64_t count)
{
	ModelBounds bounds;
	
	float min[3] = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
	float max[3] = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };

	for (int i = 0; i != count; ++i)
	{
		min[0] = std::min(min[0], vertices[i].position.x);
		min[1] = std::min(min[1], vertices[i].position.y);
		min[2] = std::min(min[2], vertices[i].position.z);

		max[0] = std::max(max[0], vertices[i].position.x);
		max[1] = std::max(max[1], vertices[i].position.y);
		max[2] = std::max(max[2], vertices[i].position.z);
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
}

void utils::set_up_basic_texture_info(TextureInfo* info)
{
	info->compressionMode = CompressionMode::LZ4;
	info->mipmapMode = MipmapMode::BASE_MIPMAPPING;
	info->runtimeCompressionMode = RuntimeCompressionMode::DXT1;
	info->tilingX = TilingMode::REPEAT;
	info->tilingY = TilingMode::REPEAT;
	info->sRGB = true;
	info->brightness = 1.0;
	info->saturation = 1.0;
}

void utils::pack_model_info(ModelInfo* modelInfo, ResourceInfo* resourceInfo)
{
	nlohmann::json modelMetaData;
	modelMetaData["uuid"] = (uint64_t)modelInfo->uuid;
	modelMetaData["original_file"] = modelInfo->originalFile;
	modelMetaData["vertex_buffer_size"] = modelInfo->vertexBufferSize;
	modelMetaData["index_buffer_size"] = modelInfo->indexBufferSize;

	std::vector<float> data;
	data.resize(7);
	data[0] = modelInfo->bounds.origin.x;
	data[1] = modelInfo->bounds.origin.y;
	data[2] = modelInfo->bounds.origin.z;
	data[3] = modelInfo->bounds.radius;
	data[4] = modelInfo->bounds.extents.x;
	data[5] = modelInfo->bounds.extents.y;
	data[6] = modelInfo->bounds.extents.z;
	modelMetaData["bounds"] = data;

	modelMetaData["vertex_format"] = get_str_vertex_format(modelInfo->vertexFormat);
	modelMetaData["compression_mode"] = get_str_compression_mode(modelInfo->compressionMode);
	modelMetaData["name"] = modelInfo->name.c_str();

	modelMetaData["materials_name"] = modelInfo->materialsName;
	
	resourceInfo->type = ResourceType::MODEL;
	resourceInfo->dataSize = modelInfo->vertexBufferSize + modelInfo->indexBufferSize;
	resourceInfo->metaData = modelMetaData.dump();
}

ModelInfo* utils::unpack_model_info(ResourceInfo* resourceInfo)
{
	ModelInfo* modelInfo = new ModelInfo();
	
	nlohmann::json modelMetaData = nlohmann::json::parse(resourceInfo->metaData);
	modelInfo->uuid = UUID(modelMetaData["uuid"]);
	modelInfo->originalFile = modelMetaData["original_file"];
	modelInfo->vertexBufferSize = modelMetaData["vertex_buffer_size"];
	modelInfo->indexBufferSize = modelMetaData["index_buffer_size"];
	modelInfo->vertexFormat = get_enum_vertex_format(modelMetaData["vertex_format"]);
	modelInfo->compressionMode = get_enum_compression_mode(modelMetaData["compression_mode"]);
	modelInfo->name = modelMetaData["name"];
	modelInfo->materialsName = modelMetaData["materials_name"].get<std::vector<std::string>>();

	std::vector<float> data;
	data.resize(7);
	data = modelMetaData["bounds"].get<std::vector<float>>();
	
	modelInfo->bounds.origin = glm::vec3(data[0], data[1], data[2]);
	modelInfo->bounds.radius = data[3];
	modelInfo->bounds.extents = glm::vec3(data[4], data[5], data[6]);
	
	return modelInfo;
}


void utils::pack_texture_info(TextureInfo* textureInfo, ResourceInfo* resourceInfo)
{
	nlohmann::json textureMetaData;
	textureMetaData["original_file"] = textureInfo->originalFile;
	textureMetaData["name"] = textureInfo->name;
	textureMetaData["texture_size"] = textureInfo->size;
	textureMetaData["texture_width"] = textureInfo->width;
	textureMetaData["texture_height"] = textureInfo->height;
	textureMetaData["compression_mode"] = get_str_compression_mode(textureInfo->compressionMode);
	textureMetaData["mipmap_mode"] = get_str_mipmap_mode(textureInfo->mipmapMode);
	textureMetaData["runtime_compression"] = get_str_runtime_compression(textureInfo->runtimeCompressionMode);
	textureMetaData["tiling_x"] = get_str_tiling_mode(textureInfo->tilingX);
	textureMetaData["tiling_y"] = get_str_tiling_mode(textureInfo->tilingY);
	textureMetaData["sRGB"] = textureInfo->sRGB;
	textureMetaData["brightness"] = textureInfo->brightness;
	textureMetaData["saturation"] = textureInfo->saturation;

	resourceInfo->type = ResourceType::TEXTURE;
	resourceInfo->dataSize = textureInfo->size;
	resourceInfo->metaData = textureMetaData.dump();
}

TextureInfo* utils::unpack_texture_info(ResourceInfo* resourceInfo)
{
	TextureInfo* textureInfo = new TextureInfo();
	
	nlohmann::json textureMetaData = nlohmann::json::parse(resourceInfo->metaData);
	textureInfo->originalFile = textureMetaData["original_file"];
	textureInfo->name = textureMetaData["name"];
	textureInfo->size = textureMetaData["texture_size"];
	textureInfo->width = textureMetaData["texture_width"];
	textureInfo->height = textureMetaData["texture_height"];
	textureInfo->compressionMode = get_enum_compression_mode(textureMetaData["compression_mode"]);
	textureInfo->mipmapMode = get_enum_mipmap_mode(textureMetaData["mipmap_mode"]);
	textureInfo->runtimeCompressionMode = get_enum_runtime_compression(textureMetaData["runtime_compression"]);
	textureInfo->tilingX = get_enum_tiling_mode(textureMetaData["tiling_x"]);
	textureInfo->tilingY = get_enum_tiling_mode(textureMetaData["tiling_y"]);
	textureInfo->sRGB = textureMetaData["sRGB"];
	textureInfo->brightness = textureMetaData["brightness"];
	textureInfo->saturation = textureMetaData["saturation"];

	return textureInfo;
}
	
void utils::pack_level_info(LevelEngineInfo* levelInfo)
{
	nlohmann::json levelMetaData;
	
	std::vector<float> data;
	data.resize(3);

	for (auto& modelUUID : levelInfo->modelInstancesInfo)
	{
		std::vector<nlohmann::json> modelInstacesMetaData;
		for (auto& instanceInfo : modelUUID.second)
		{
			nlohmann::json modelInstanceMetaData;
			data[0] = instanceInfo.translation.x;
			data[1] = instanceInfo.translation.y;
			data[2] = instanceInfo.translation.z;
			modelInstanceMetaData["translation"] = data;
			data[0] = instanceInfo.rotationAxis.x;
			data[1] = instanceInfo.rotationAxis.y;
			data[2] = instanceInfo.rotationAxis.z;
			modelInstanceMetaData["rotation_axis"] = data;
			modelInstanceMetaData["rotation_angle"] = instanceInfo.rotationAngle;
			data[0] = instanceInfo.scale.x;
			data[1] = instanceInfo.scale.y;
			data[2] = instanceInfo.scale.z;
			modelInstanceMetaData["scale"] = data;

			modelInstanceMetaData["type"] = get_str_model_type(instanceInfo.type);
			modelInstanceMetaData["is_shadow_casted"] = instanceInfo.isShadowCasted;

			std::vector<uint64_t> materialsUUID;
			for (auto& uuid : instanceInfo.engineMaterials)
			{
				materialsUUID.push_back(uuid);
			}

			modelInstanceMetaData["engine_materials_uuid"] = materialsUUID;
		}

		levelMetaData[std::to_string(modelUUID.first)] = modelInstacesMetaData;
	}

	std::vector<uint64_t> texturesUUID;
	for (auto& textureUUID : levelInfo->texturesUUID)
	{
		texturesUUID.push_back(textureUUID);
	}
	levelMetaData["textures_uuid"] = texturesUUID;
}

LevelEngineInfo utils::unpack_level_info(ResourceInfo* resourceInfo)
{
	// modelInfo->type = get_enum_model_type(modelMetaData["type"]);
	// modelInfo->isShadowCasted = modelMetaData["is_shadow_casted"];
	// modelInfo->materialsName = modelMetaData["materials_name"].get<std::vector<std::string>>();
	//
	// std::vector<float> data;
	// data.resize(7);
	// data = modelMetaData["bounds"].get<std::vector<float>>();
	//
	// modelInfo->bounds.origin = glm::vec3(data[0], data[1], data[2]);
	// modelInfo->bounds.radius = data[3];
	// modelInfo->bounds.extents = glm::vec3(data[4], data[5], data[6]);
	//
	// data.resize(3);
	// data = modelMetaData["translation"].get<std::vector<float>>();
	// modelInfo->translation = glm::vec3(data[0], data[1], data[2]);
	// data = modelMetaData["rotation_axis"].get<std::vector<float>>();
	// modelInfo->rotationAxis = glm::vec3(data[0], data[1], data[2]);
	// data = modelMetaData["scale"].get<std::vector<float>>();
	// modelInfo->scale = glm::vec3(data[0], data[1], data[2]);
	// modelInfo->rotationAngle = modelMetaData["rotation_angle"];
	//
	// std::vector<uint64_t> uuids;
	// uuids = modelMetaData["engine_materials_uuid"].get<std::vector<uint64_t>>();
	// for (auto& uuid : uuids)
	// {
	// 	modelInfo->engineMaterials.push_back(uuid);
	// }
	//
	// modelInfo->data = resourceInfo->data;
	// TODO
	LevelEngineInfo levelEngineInfo;
	return levelEngineInfo;
}
