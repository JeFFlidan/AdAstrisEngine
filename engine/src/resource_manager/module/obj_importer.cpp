#include "obj_importer.h"
#include "core/utils.h"
#include <tinyobjloader/tiny_obj_loader.h>

using namespace ad_astris;
using namespace resource::impl;

bool OBJImporter::import(
	const std::string& path,
	std::vector<ModelCreateInfo>& outModelInfos,
	std::vector<TextureCreateInfo>& outTextureInfos,
	std::vector<MaterialCreateInfo>& materialInfos,
	const ecore::ModelConversionContext& conversionContext)
{
	// TODO think how to fill this config using conversion context
	tinyobj::ObjReaderConfig config;
	config.triangulate = false;
	config.vertex_color = false;

	tinyobj::ObjReader reader;
	LOG_INFO("BEFORE PARSE")
	reader.ParseFromFile(path, config);
	LOG_INFO("AFTER PARSE")
	if (!reader.Warning().empty())
	{
		LOG_WARNING("OBJImporter::import(): {}", reader.Warning())
	}
	
	if (!reader.Valid())
	{
		if (!reader.Error().empty())
		{
			LOG_ERROR("OBJImporter::import: {}", reader.Error())
			return false;
		}
	}

	// Can't use vector because of the strange behavior of LZ4 compression
	const std::vector<tinyobj::shape_t> shapes = reader.GetShapes();
	const std::vector<tinyobj::material_t> materials = reader.GetMaterials();
	const tinyobj::attrib_t& attrib = reader.GetAttrib();

	for (auto& material : materials)
	{
		// TODO
	}

	ModelCreateInfo mergedModelCreateInfo;
	LOG_INFO("BEFORE LOOP {} {}", shapes.size(), shapes[0].mesh.indices.size())
	for (auto& shape : shapes)
	{
		ModelCreateInfo& outModelCreateInfo = conversionContext.mergeMeshes ? mergedModelCreateInfo : outModelInfos.emplace_back();
		if (!conversionContext.mergeMeshes)
			outModelCreateInfo.name = shape.name;
		std::unordered_map<uint64_t, uint32_t> modelIndexByVertex;
		for (size_t i = 0; i < shape.mesh.indices.size(); i += 3)
		{
			tinyobj::index_t indices[3] = {
				shape.mesh.indices[i + 0],
				shape.mesh.indices[i + 1],
				shape.mesh.indices[i + 2]
			};

			for (auto& index : indices)
			{
				XMFLOAT3 position = XMFLOAT3{
					attrib.vertices[index.vertex_index * 3 + 0],
					attrib.vertices[index.vertex_index * 3 + 1],
					attrib.vertices[index.vertex_index * 3 + 2]
				};

				XMFLOAT3 normal(0, 0, 0);
				if (!attrib.normals.empty())
				{
					normal = XMFLOAT3{
						attrib.normals[index.normal_index * 3 + 0],
						attrib.normals[index.normal_index * 3 + 1],
						attrib.normals[index.normal_index * 3 + 2]
					};
				}

				XMFLOAT2 uv(0, 0);
				if (!attrib.texcoords.empty())
				{
					uv = XMFLOAT2{
						attrib.texcoords[index.texcoord_index * 2 + 0],
						attrib.texcoords[index.texcoord_index * 2 + 1]	// Invert?
					};
				}

				uint64_t vertexHash = 0;
				CoreUtils::hash_combine(vertexHash, index.vertex_index);
				CoreUtils::hash_combine(vertexHash, index.normal_index);
				CoreUtils::hash_combine(vertexHash, index.texcoord_index);

				if (modelIndexByVertex.find(vertexHash) == modelIndexByVertex.end())
				{
					modelIndexByVertex[vertexHash] = (uint32_t)outModelCreateInfo.info.vertexPositions.size();
					outModelCreateInfo.info.vertexPositions.push_back(position);
					outModelCreateInfo.info.vertexNormals.push_back(normal);
					outModelCreateInfo.info.vertexUVSet0.push_back(uv);
					// TODO add mesh with material index
				}
				outModelCreateInfo.info.indices.push_back(modelIndexByVertex[vertexHash]);
			}
		}
	}

	if (conversionContext.mergeMeshes)
		outModelInfos.push_back(mergedModelCreateInfo);
	
	return true;
}
