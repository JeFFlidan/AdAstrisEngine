#include "model_common.h"
#include "engine_core/object.h"
#include <json.hpp>

using namespace ad_astris::ecore;

std::string model::Utils::get_str_vertex_format(VertexFormat format)
{
	switch (format)
	{
		case VertexFormat::F32_PNTC:
			return "f32_pntc";
		case VertexFormat::UNKNOWN:
			return "unknown";
	}
}

model::VertexFormat model::Utils::get_enum_vertex_format(std::string format)
{
	if (format == "f32_pntc")
		return VertexFormat::F32_PNTC;
	if (format == "unknown")
		return VertexFormat::UNKNOWN;
}

std::string model::Utils::pack_static_model_info(StaticModelInfo* info)
{
	nlohmann::json modelMetaData;
	modelMetaData["uuid"] = (uint64_t)info->uuid;
	modelMetaData["original_file"] = info->originalFile;
	modelMetaData["vertex_buffer_size"] = info->vertexBufferSize;
	modelMetaData["index_buffer_size"] = info->indexBufferSize;
	std::vector<float> data;
	data.resize(7);
	data[0] = info->bounds.origin.x;
	data[1] = info->bounds.origin.y;
	data[2] = info->bounds.origin.z;
	data[3] = info->bounds.radius;
	data[4] = info->bounds.extents.x;
	data[5] = info->bounds.extents.y;
	data[6] = info->bounds.extents.z;
	modelMetaData["bounds"] = data;
	modelMetaData["vertex_format"] = get_str_vertex_format(info->vertexFormat);
	modelMetaData["materials_name"] = info->materialsName;

	return modelMetaData.dump();
}

model::StaticModelInfo model::Utils::unpack_static_model_info(std::string& strMetaData)
{
	nlohmann::json metaData = nlohmann::json::parse(strMetaData);
	StaticModelInfo info;
	info.uuid = UUID(metaData["uuid"]);
	info.originalFile = metaData["original_file"];
	info.vertexBufferSize = metaData["vertex_buffer_size"];
	info.indexBufferSize = metaData["index_buffer_size"];
	info.vertexFormat = get_enum_vertex_format(metaData["vertex_format"]);
	info.materialsName = metaData["materials_name"].get<std::vector<std::string>>();

	std::vector<float> data;
	data.resize(7);
	data = metaData["bounds"].get<std::vector<float>>();
	
	info.bounds.origin = glm::vec3(data[0], data[1], data[2]);
	info.bounds.radius = data[3];
	info.bounds.extents = glm::vec3(data[4], data[5], data[6]);

	return info;
}

model::ModelBounds model::Utils::calculate_model_bounds(VertexF32PNTC* vertices, uint64_t count)
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

void model::Utils::calculate_tangent(VertexF32PNTC* vertices)
{
	VertexF32PNTC& firstVert = vertices[0];
	VertexF32PNTC& secondVert = vertices[1];
	VertexF32PNTC& thirdVert = vertices[2];
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

void model::Utils::setup_f32_pntc_format_description(
	std::vector<rhi::VertexBindingDescription>& bindingDescriptions,
	std::vector<rhi::VertexAttributeDescription>& attributeDescriptions)
{
	rhi::VertexBindingDescription vertexBindingDesc;
	vertexBindingDesc.binding = 0;
	vertexBindingDesc.stride = sizeof(VertexF32PNTC);
	bindingDescriptions.push_back(vertexBindingDesc);

	rhi::VertexAttributeDescription positionAttributeDesc;
	positionAttributeDesc.binding = 0;
	positionAttributeDesc.format = rhi::Format::R32G32B32A32_SFLOAT;
	positionAttributeDesc.location = 0;
	positionAttributeDesc.offset = offsetof(VertexF32PNTC, position);
	attributeDescriptions.push_back(positionAttributeDesc);

	rhi::VertexAttributeDescription normalAttributeDesc;
	normalAttributeDesc.binding = 0;
	normalAttributeDesc.format = rhi::Format::R32G32B32A32_SFLOAT;
	normalAttributeDesc.location = 1;
	normalAttributeDesc.offset = offsetof(VertexF32PNTC, normal);
	attributeDescriptions.push_back(normalAttributeDesc);

	rhi::VertexAttributeDescription tangentAttributeDesc;
	tangentAttributeDesc.binding = 0;
	tangentAttributeDesc.format = rhi::Format::R32G32B32A32_SFLOAT;
	tangentAttributeDesc.location = 2;
	tangentAttributeDesc.offset = offsetof(VertexF32PNTC, tangent);
	attributeDescriptions.push_back(tangentAttributeDesc);
	
	rhi::VertexAttributeDescription texCoordAttributeDesc;
	texCoordAttributeDesc.binding = 0;
	texCoordAttributeDesc.format = rhi::Format::R32G32B32A32_SFLOAT;
	texCoordAttributeDesc.location = 3;
	texCoordAttributeDesc.offset = offsetof(VertexF32PNTC, texCoord);
	attributeDescriptions.push_back(texCoordAttributeDesc);
}

void model::Utils::setup_f32_pc_format_description(
	std::vector<rhi::VertexBindingDescription>& bindingDescriptions,
	std::vector<rhi::VertexAttributeDescription>& attributeDescriptions)
{
	rhi::VertexBindingDescription vertexBindingDesc;
	vertexBindingDesc.binding = 0;
	vertexBindingDesc.stride = sizeof(VertexF32PC);
	bindingDescriptions.push_back(vertexBindingDesc);

	rhi::VertexAttributeDescription positionAttributeDesc;
	positionAttributeDesc.binding = 0;
	positionAttributeDesc.format = rhi::Format::R32G32B32A32_SFLOAT;
	positionAttributeDesc.location = 0;
	positionAttributeDesc.offset = offsetof(VertexF32PC, position);
	attributeDescriptions.push_back(positionAttributeDesc);

	rhi::VertexAttributeDescription texCoordAttributeDesc;
	texCoordAttributeDesc.binding = 0;
	texCoordAttributeDesc.format = rhi::Format::R32G32B32A32_SFLOAT;
	texCoordAttributeDesc.location = 1;
	texCoordAttributeDesc.offset = offsetof(VertexF32PC, texCoord);
	attributeDescriptions.push_back(texCoordAttributeDesc);
}

