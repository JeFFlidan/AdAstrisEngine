#include "nlohmann/detail/conversions/to_json.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>

#include <json.hpp>
#include <lz4.h>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <asset_loader.h>
#include <material_asset.h>
#include <prefab_asset.h>
#include <texture_asset.h>
#include <mesh_asset.h>
#include <tiny_obj_loader.h>

#include <logger.h>

namespace fs = std::filesystem;

using Vertex = assets::Vertex_f32_PNCV;

Vertex pack_vertex(tinyobj::real_t vx,
	tinyobj::real_t vy,
	tinyobj::real_t vz,
	tinyobj::real_t nx,
	tinyobj::real_t ny,
	tinyobj::real_t nz,
	tinyobj::real_t ux,
	tinyobj::real_t uy
)
{
	Vertex new_vert;

	new_vert.position[0] = vx;
	new_vert.position[1] = vy;
	new_vert.position[2] = vz;

	new_vert.normal[0] = nx;
	new_vert.normal[1] = ny;
	new_vert.normal[2] = nz;

	//new_vert.color[0] = 0.0f;
	//new_vert.color[1] = 0.0f;
	//new_vert.color[2] = 0.0f;

	new_vert.uv[0] = ux;
	new_vert.uv[1] = 1 - uy;

	return new_vert;
}

void load_obj_file(tinyobj::attrib_t& attrib, 
	std::vector<tinyobj::shape_t>& shapes, 
	std::vector<Vertex>& vertices, 
	std::vector<uint32_t>& indices
)
{
	std::vector<Vertex> threeVertices;
	for (size_t s = 0; s != shapes.size(); ++s)
	{
		size_t index_offset = 0;
		for (size_t f = 0; f != shapes[s].mesh.num_face_vertices.size(); ++f)
		{
			int fv = 3;

			for (size_t v = 0; v != fv; ++v)
			{
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

				tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
				tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
				tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

				tinyobj::real_t ux = attrib.texcoords[2 * idx.texcoord_index + 0];
				tinyobj::real_t uy = attrib.texcoords[2 * idx.texcoord_index + 1];

				Vertex new_vert = pack_vertex(vx, vy, vz, nx, ny, nz, ux, uy);

				threeVertices.push_back(new_vert);

				if (threeVertices.size() == 3)
				{
					Vertex firstVert = threeVertices[0];
					Vertex secondVert = threeVertices[1];
					Vertex thirdVert = threeVertices[2];
					float vertPos1[3] = { firstVert.position[0], firstVert.position[1], firstVert.position[2] };
					float vertPos2[3] = { secondVert.position[0], secondVert.position[1], secondVert.position[2] };
					float vertPos3[3] = { thirdVert.position[0], thirdVert.position[1], thirdVert.position[2] };
					float vertUv1[2] = { firstVert.uv[0], firstVert.uv[1] };
					float vertUv2[2] = { secondVert.uv[0], secondVert.uv[1] };
					float vertUv3[2] = { thirdVert.uv[0], thirdVert.uv[1] };
					float edge1[3] = { vertPos2[0] - vertPos1[0], vertPos2[1] - vertPos1[1], vertPos2[2] - vertPos1[2] };
					float edge2[3] = { vertPos3[0] - vertPos1[0], vertPos3[1] - vertPos1[1], vertPos3[2] - vertPos1[2] };
					float deltaUV1[2] = { vertUv2[0] - vertUv1[0], vertUv2[1] - vertUv1[1] };
					float deltaUV2[2] = { vertUv3[0] - vertUv1[0], vertUv3[1] - vertUv1[1] };
					float f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);
					float tangent[3];
					tangent[0] = f * (deltaUV2[1] * edge1[0] - deltaUV1[1] * edge2[0]);
					tangent[1] = f * (deltaUV2[1] * edge1[1] - deltaUV1[1] * edge2[1]);
					tangent[2] = f * (deltaUV2[1] * edge1[2] - deltaUV1[1] * edge2[2]);
					firstVert.tangent[0] = tangent[0];
					firstVert.tangent[1] = tangent[1];
					firstVert.tangent[2] = tangent[2];
					secondVert.tangent[0] = tangent[0];
					secondVert.tangent[1] = tangent[1];
					secondVert.tangent[2] = tangent[2];
					thirdVert.tangent[0] = tangent[0];
					thirdVert.tangent[1] = tangent[1];
					thirdVert.tangent[2] = tangent[2];
					indices.push_back(vertices.size());
					vertices.push_back(firstVert);
					indices.push_back(vertices.size());
					vertices.push_back(secondVert);
					indices.push_back(vertices.size());
					vertices.push_back(thirdVert);
					threeVertices.clear();
				}

				//indices.push_back(vertices.size());
				//vertices.push_back(new_vert);
			}

			index_offset += fv;
		}
	}
}

bool convert_mesh(const fs::path& input, const fs::path& output)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, input.string().c_str(), nullptr);

	if (!warn.empty())
	{
		LOG_WARNING("Warning when loading material from obj file {}", warn);
	}

	if (!err.empty())
	{
		std::cerr << "ERROR: " << err << std::endl;
		return false;
	}

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	
	load_obj_file(attrib, shapes, vertices, indices);

	assets::MeshBounds meshBounds = assets::calculate_bounds(vertices.data(), vertices.size());

	assets::MeshInfo meshInfo;
	meshInfo.vertexBufferSize = vertices.size() * sizeof(Vertex);
	meshInfo.indexBufferSize = indices.size() * sizeof(uint32_t);
	meshInfo.vertexFormat = assets::VertexFormat::PNCV_F32;
	meshInfo.indexSize = 1;
	meshInfo.originalFile = input.string();
	meshInfo.bounds = meshBounds;

	assets::AssetFile newMesh = assets::pack_mesh(&meshInfo, (char*)vertices.data(), (char*)indices.data());

	assets::save_binaryFile(output.string().c_str(), newMesh);

	return true;
}

bool convert_image_with_alpha(const fs::path& input, const fs::path& output)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(input.u8string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	std::cout << "Texture channels " << texChannels;
	if (!pixels)
	{
		LOG_ERROR("Failed to load texture file {}", input.u8string().c_str());
		return false;
	}

	int texture_size = texHeight * texWidth * 4;

	assets::TextureInfo texInfo;
	texInfo.textureSize = texture_size;
	texInfo.textureFormat = assets::TextureFormat::RGBA8;
	texInfo.pixelSize[0] = texWidth;
	texInfo.pixelSize[1] = texHeight;
	texInfo.originalFile = input.string();
	assets::AssetFile newImage = assets::pack_texture(&texInfo, pixels);

	stbi_image_free(pixels);

	assets::save_binaryFile(output.string().c_str(), newImage);

	return true;
}

bool convert_image_without_alpha(const fs::path& input, const fs::path& output)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(input.u8string().c_str(), &texWidth, &texHeight, &texChannels, 0);
	std::cout << "Texture channels " << texChannels << std::endl;
	if (!pixels)
	{
		LOG_ERROR("Failed to load texture file {}", input.u8string().c_str());
		return false;
	}

	int texture_size = texHeight * texWidth * texChannels;

	std::cout << "start" << std::endl;
	assets::TextureInfo texInfo;
	texInfo.textureSize = texture_size;
	texInfo.textureFormat = assets::TextureFormat::RGB8;
	texInfo.pixelSize[0] = texWidth;
	texInfo.pixelSize[1] = texHeight;
	texInfo.originalFile = input.string();
	assets::AssetFile newImage = assets::pack_texture(&texInfo, pixels);
	std::cout << "end" << std::endl;

	stbi_image_free(pixels);

	assets::save_binaryFile(output.string().c_str(), newImage);

	return true;
}

// Temporary solution, it must be replaced by saving materials from the engine for the particular project
void save_material(
	fs::path directory,
	const std::string& materialName,
	const std::string& baseEffect,
	const std::vector<std::string>& textures,
	assets::MaterialMode materialMode)
{
	// In the textures vector: 0 index is base color, 1 - normal, 2 - arm (if it's a PBR shader, but in other cases the order will be the same)
	assets::MaterialInfo info;
	info.materialName = materialName;
	info.baseEffect = baseEffect;
	if (!textures.empty())
	{
		info.textures["texture_base_color"] = directory.string() + textures[0];
		info.textures["texture_normal"] = directory.string() + textures[1];
		info.textures["texture_arm"] = directory.string() + textures[2];
	}
    info.mode = materialMode;

    assets::AssetFile file = assets::pack_material(&info);
    std::string path = directory.string()+ '/' + materialName + ".mat";
    assets::save_binaryFile(path.c_str(), file);
}

// Temporary solution, it must be replaced by saving prefabs from the engine for the particular project
void save_prefab(const std::string& directory, const std::string& prefabName)
{
	assets::PrefabInfo info;
	std::array<float, 16> matrix;
	for (int i = 0; i != 16; ++i)
	{
		matrix[i] = 1.0f;
	}
	info.matrix = matrix;
	info.materialPath = directory + '/' + prefabName + ".mat";
	info.meshPath = directory + '/' +  prefabName + ".mesh";

	assets::AssetFile file = assets::pack_prefab(&info);
	assets::save_binaryFile((directory + '/' + prefabName + ".pref").c_str(), file);
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		LOG_WARNING("You need to put the path to the info file");
		return -1;
	}
	else
	{
		fs::path path{ argv[1] };
		fs::path directory = path;
		LOG_INFO("loading asset directory at {}", directory.string().c_str());
		for (auto& p : fs::directory_iterator(directory))
		{
			LOG_INFO("File {}", p.path().string().c_str());
			if (p.path().extension() == ".png")
			{
				auto newpath = p.path();
				newpath.replace_extension(".tx");
				auto jsonPath = p.path();
				jsonPath.replace_extension(".json");
				convert_image_with_alpha(p.path(), newpath);
			}
			else if(p.path().extension() == ".tga")
			{
				auto newpath = p.path();
				newpath.replace_extension(".tx");
				convert_image_with_alpha(p.path(), newpath);			  
			}
			else if (p.path().extension() == ".obj")
			{
				auto newpath = p.path();
				newpath.replace_extension(".mesh");
				auto jsonPath = p.path();
				jsonPath.replace_extension(".json");
				convert_mesh(p.path(), newpath);
			}
		}

		// Temporary solution, should be removed in the future.
		if (argc == 3 && strcmp("create_prefab", argv[2]) == 0)
		{
			save_material(
				directory,
				"door",
				"PBR_opaque",
				{ "/Sci fi door 1_BaseColor.tx", "/Sci fi door 1_Normal.tx", "/Sci fi door 1_ARM.tx" },
				assets::MaterialMode::OPAQUE
			);

			save_material(
				directory,
				"gun",
				"PBR_opaque",
				{ "/Gun_BaseColor.tx", "/Gun_Normal.tx", "/Gun_ARM.tx" },
				assets::MaterialMode::OPAQUE
			);

			save_material(
				directory,
				"gun2",
				"PBR_opaque",
				{ "/Gun_2_BaseColor.tx", "/Gun_2_Normal.tx", "/Gun_2_ARM.tx" },
				assets::MaterialMode::OPAQUE
			);

			save_material(
				directory,
				"wall",
				"PBR_opaque",
				{ "/Painted_metal_basecolor.tx", "/Painted_metal_normal.tx", "/Painted_metal_ARM.tx" },
				assets::MaterialMode::OPAQUE
			);

			//save_prefab(directory.string(), "gun");
			//save_prefab(directory.string(), "gun2");
			save_prefab(directory.string(), "door");
			save_prefab(directory.string(), "wall");
		}
	}
}
