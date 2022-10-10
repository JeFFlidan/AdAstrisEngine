#include <iostream>
#include <fstream>
#include <filesystem>

#include <json.hpp>
#include <lz4.h>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <asset_loader.h>
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

	new_vert.color[0] = 0.0f;
	new_vert.color[1] = 0.0f;
	new_vert.color[2] = 0.0f;

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

				indices.push_back(vertices.size());
				vertices.push_back(new_vert);
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

	assets::MeshInfo meshInfo;
	meshInfo.vertexBufferSize = vertices.size() * sizeof(Vertex);
	meshInfo.indexBufferSize = indices.size() * sizeof(uint32_t);
	meshInfo.vertexFormat = assets::VertexFormat::PNCV_F32;
	meshInfo.indexSize = 1;
	meshInfo.originalFile = input.string();

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
				convert_image_without_alpha(p.path(), newpath);			  
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
	}
}
