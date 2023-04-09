#include "resource_converter.h"
#include "profiler/logger.h"
#include "utils.h"

#include <tiny_obj_loader.h>
#ifndef STB_IMAGE_IMPLEMENTATION
	#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb_image.h>
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#include <complex>
#include <lz4.h>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>

using namespace ad_astris;

resource::ResourceConverter::ResourceConverter(io::FileSystem* fileSystem) : _fileSystem(fileSystem)
{
	
}

void* resource::ResourceConverter::convert_to_aares_file(io::URI& path, void* existedInfo)
{
	std::string fileExt(std::filesystem::path(path.c_str()).extension().string().erase(0, 1));

	if (fileExt == "glb" || fileExt == "gltf" || fileExt == "obj")
	{
		ModelInfo* modelInfo;
		if (existedInfo != nullptr)
		{
			modelInfo = static_cast<ModelInfo*>(existedInfo);
		}
		else
		{
			modelInfo = new ModelInfo();
			utils::set_up_basic_model_info(modelInfo);
			modelInfo->name = utils::get_resource_name(path);
			modelInfo->originalFile = path.c_str();
		}
		
		if (fileExt == "glb" || fileExt == "gltf")
		{
			convert_to_model_info_from_gltf(path, modelInfo);
			write_info_to_disk(modelInfo);
		}
		else if (fileExt == "obj")
		{
			convert_to_model_info_from_obj(path, modelInfo);
			write_info_to_disk(modelInfo);
		}
		return modelInfo;
	}
	
	if (fileExt == "tga" || fileExt == "png")
	{
		TextureInfo* textureInfo;
		if (existedInfo != nullptr)
		{
			textureInfo = static_cast<TextureInfo*>(existedInfo);
		}
		else
		{
			textureInfo = new TextureInfo();
			utils::set_up_basic_texture_info(textureInfo);
			textureInfo->name = utils::get_resource_name(path);
			textureInfo->originalFile = path.c_str();
		}
		convert_to_texture_info_from_raw_image(path, textureInfo);
		write_info_to_disk(textureInfo);
		return textureInfo;
	}
	
	LOG_ERROR("Unsupported file format")
	return nullptr;
}

void resource::ResourceConverter::write_info_to_disk(ModelInfo* modelInfo)
{
	ResourceInfo resourceInfo = utils::pack_model_info(modelInfo);
	
	io::URI path = std::string("assets/" + modelInfo->name + ".aares").c_str();		// temporary solution
	
	write_to_disk(path, resourceInfo);
}

void resource::ResourceConverter::write_info_to_disk(TextureInfo* textureInfo)
{
	ResourceInfo resourceInfo = utils::pack_texture_info(textureInfo);
	
	io::URI path = std::string("assets/" + textureInfo->name + ".aares").c_str();		// temporary solution
	
	write_to_disk(path, resourceInfo);
}

void resource::ResourceConverter::convert_to_model_info_from_gltf(io::URI& path, ModelInfo* modelInfo)
{
	cgltf_options options;
	memset(&options, 0, sizeof(cgltf_options));
	cgltf_data* data = nullptr;
	cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);
	assert(result == cgltf_result_success && "Failed to parse gltf file");

	size_t materialsCount = data->materials_count;
	cgltf_material* materials = data->materials;
	for (size_t i = 0; i != materialsCount; ++i)
	{
		modelInfo->materialsName.push_back(materials[i].name);
	}
	
	result = cgltf_load_buffers(&options, data, path.c_str());
	assert(result == cgltf_result_success && "Failed to load buffers");

	std::vector<VertexF32> vertexData;
	std::vector<uint32_t> modelIndices;
	
	cgltf_mesh* meshes = data->meshes;
	size_t meshesCount = data->meshes_count;
	std::string generator(data->asset.generator);

	// I need this information to swap z and y. Maybe, I'll find another solution 
	bool isBlender = false;
	if (generator.find("Blender") != std::string::npos)
	{
		isBlender = true;
	}

	for (size_t i = 0; i != meshesCount; ++i)
	{
		// I suppose that one mesh can have only one primitive. Maybe I'll change it
		cgltf_primitive* primitives = meshes[i].primitives;
		size_t primitivesCount = meshes[i].primitives_count;
		for (size_t j = 0; j != primitivesCount; ++j)
		{
			cgltf_primitive& primitive = primitives[j];
			cgltf_attribute* attributes = primitive.attributes;
			size_t attributesCount = primitives->attributes_count;

			glm::vec3* position = nullptr;
			glm::vec3* normal = nullptr;
			glm::vec2* texCoord = nullptr;
			size_t count = 0;
			for (size_t q = 0; q != attributesCount; ++q)
			{
				cgltf_attribute& attribute = attributes[q];
				switch (attribute.type)
				{
					case cgltf_attribute_type_position:
					{
						cgltf_accessor* accessor = attribute.data;
						position = new glm::vec3[accessor->count];
						cgltf_buffer_view* bufferView = accessor->buffer_view;
						float* bufferData = static_cast<float*>(bufferView->buffer->data);
						bufferData += bufferView->offset / sizeof(float);

						memcpy(position, bufferData, bufferView->size);

						count = accessor->count;

						break;
					}
					case cgltf_attribute_type_normal:
					{
						cgltf_accessor* accessor = attribute.data;
						normal = new glm::vec3[accessor->count];

						cgltf_buffer_view* bufferView = accessor->buffer_view;
						float* bufferData = static_cast<float*>(bufferView->buffer->data);
						bufferData += bufferView->offset / sizeof(float);

						memcpy(normal, bufferData, bufferView->size);

						break;
					}
					case cgltf_attribute_type_texcoord:
					{
						cgltf_accessor* accessor = attribute.data;
						texCoord = new glm::vec2[accessor->count];
						
						cgltf_buffer_view* bufferView = accessor->buffer_view;
						float* bufferData = static_cast<float*>(bufferView->buffer->data);
						bufferData += bufferView->offset / sizeof(float);
						
						memcpy(texCoord, bufferData, bufferView->size);

						break;
					}
				}
			}

			VertexF32 threeVertices[3];
			size_t index = 0;
			for (size_t g = 0; g != count; ++g)
			{
				VertexF32 vertex;
				vertex.position.x = position[g].x;
				vertex.normal.x = normal[g].x;
				if (isBlender)
				{
					vertex.position.y = position[g].z;
					vertex.position.z = position[g].y;
					vertex.normal.y = normal[g].z;
					vertex.normal.z = normal[g].y;
				}
				else
				{
					vertex.position.y = position[g].y;
					vertex.position.z = position[g].z;
					vertex.normal.y = normal[g].y;
					vertex.normal.z = normal[g].z;
				}
				vertex.texCoord.x = texCoord[g].x;
				vertex.texCoord.y = texCoord[g].y;
				threeVertices[index] = vertex;
				++index;
				if (index == 3)
				{
					utils::calculate_tangent(threeVertices);
					vertexData.push_back(threeVertices[0]);
					vertexData.push_back(threeVertices[1]);
					vertexData.push_back(threeVertices[2]);
					index = 0;
				}
			}

			cgltf_accessor* indicesAccessor = primitive.indices;
			cgltf_buffer_view* indicesBufferView = indicesAccessor->buffer_view;

			switch (indicesAccessor->component_type)
			{
				case cgltf_component_type_r_16u:
				{
					unsigned short* buffer = static_cast<unsigned short*>(indicesBufferView->buffer->data);
					buffer += indicesBufferView->offset / sizeof(unsigned short);
					for (size_t x = 0; x != indicesAccessor->count; ++x)
					{
						modelIndices.push_back(buffer[x]);
					}
					break;
				}
				case cgltf_component_type_r_32u:
				{
					uint32_t* buffer = static_cast<uint32_t*>(indicesBufferView->buffer->data);
					buffer += indicesBufferView->offset / sizeof(uint32_t);
					for (size_t x = 0; x != indicesAccessor->count; ++x)
					{
						modelIndices.push_back(buffer[x]);
					}
					break;
				}
			}

			delete[] position;
			delete[] normal;
			delete[] texCoord;
		}
	}

	modelInfo->bounds = utils::calculate_model_bounds(vertexData.data(), vertexData.size());
	
	uint64_t vertexBufferSize = vertexData.size() * sizeof(VertexF32);
	uint64_t indexBufferSize = modelIndices.size() * sizeof(uint32_t);
	modelInfo->vertexBufferSize = vertexBufferSize;
	modelInfo->indexBufferSize = indexBufferSize;
	uint8_t* modelData = new uint8_t[vertexBufferSize + indexBufferSize];
	memcpy(modelData, vertexData.data(), vertexBufferSize);
	memcpy(modelData + vertexBufferSize, modelIndices.data(), indexBufferSize);
	modelInfo->modelData = modelData;
}

void resource::ResourceConverter::convert_to_model_info_from_obj(io::URI& path, ModelInfo* modelInfo)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;
	
	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), nullptr);

	if (!warn.empty())
	{
		LOG_WARNING("Warning when loading material from obj file {}", warn)
	}

	if (!err.empty())
	{
		LOG_ERROR("ResourceConverter::convert_to_model_info_from_obj: {}", err.c_str())
		return;
	}

	// Can't use vector because of the strange behavior of LZ4 compression
	VertexF32 threeVertices[3];
	size_t index = 0;
	std::vector<VertexF32> vertexData;
	std::vector<uint32_t> indices;

	for (size_t i = 0; i != shapes.size(); ++i)
	{
		size_t indexOffset = 0;
		for (size_t j = 0; j != shapes[i].mesh.num_face_vertices.size(); ++j)
		{
			int fv = 3;
			for (size_t q = 0; q != fv; ++q)
			{
				tinyobj::index_t idx = shapes[i].mesh.indices[indexOffset + q];

				VertexF32 vertex;

				vertex.position.x = attrib.vertices[3 * idx.vertex_index + 0];
				vertex.position.y = attrib.vertices[3 * idx.vertex_index + 1];
				vertex.position.z = attrib.vertices[3 * idx.vertex_index + 2];

				vertex.normal.x = attrib.normals[3 * idx.normal_index + 0];
				vertex.normal.y = attrib.normals[3 * idx.normal_index + 1];
				vertex.normal.z = attrib.normals[3 * idx.normal_index + 2];

				vertex.texCoord.x = attrib.texcoords[2 * idx.texcoord_index + 0];
				vertex.texCoord.y = attrib.texcoords[2 * idx.texcoord_index + 1];

				threeVertices[index] = vertex;
				++index;
				if (index == 3)
				{
					utils::calculate_tangent(threeVertices);
					indices.push_back(vertexData.size());
					vertexData.push_back(threeVertices[0]);
					indices.push_back(vertexData.size());
					vertexData.push_back(threeVertices[1]);
					indices.push_back(vertexData.size());
					vertexData.push_back(threeVertices[2]);
					index = 0;
				}
			}
		}
	}

	for (auto& material : materials)
	{
		modelInfo->materialsName.push_back(material.name);
	}

	modelInfo->bounds = utils::calculate_model_bounds(vertexData.data(), vertexData.size());

	uint64_t vertexBufferSize = vertexData.size() * sizeof(VertexF32);
	uint64_t indexBufferSize = indices.size() * sizeof(uint32_t);
	modelInfo->vertexBufferSize = vertexBufferSize;
	modelInfo->indexBufferSize = indexBufferSize;
	uint8_t* modelData = new uint8_t[vertexBufferSize + indexBufferSize];
	memcpy(modelData, vertexData.data(), vertexBufferSize);
	memcpy(modelData + vertexBufferSize, indices.data(), indexBufferSize);
	modelInfo->modelData = modelData;
}

void resource::ResourceConverter::convert_to_texture_info_from_raw_image(io::URI& path, TextureInfo* texInfo)
{
	int32_t texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	texInfo->size = texWidth * texHeight * 4;
	texInfo->width = texWidth;
	texInfo->height = texHeight;

	uint8_t* data = new uint8_t[texInfo->size];
	memcpy(data, pixels, texInfo->size);
	texInfo->data = data;
	stbi_image_free(pixels);
}

void resource::ResourceConverter::write_to_disk(io::URI& path, ResourceInfo& resourceInfo)
{
	int compressStaging = LZ4_compressBound(resourceInfo.dataSize);
	std::vector<char> binaryBlob;
	binaryBlob.resize(compressStaging);
	int compressedSize = LZ4_compress_default(
		(const char*)resourceInfo.data,
		binaryBlob.data(),
		resourceInfo.dataSize,
		compressStaging);
	
	uint64_t metaDataLength = resourceInfo.metaData.size();
	std::string resourceType = utils::get_str_resource_type(resourceInfo.type);
	uint8_t resourceTypeLength = resourceType.size();
	
	io::Stream* stream = _fileSystem->open(path, "wb");

	stream->write(&resourceTypeLength, sizeof(uint8_t), 1);
	stream->write(resourceType.data(), sizeof(char), resourceTypeLength);
	stream->write(&metaDataLength, sizeof(uint64_t), 1);
	stream->write(&compressedSize, sizeof(uint64_t), 1);
	stream->write(resourceInfo.metaData.data(), sizeof(char), metaDataLength);
	stream->write(binaryBlob.data(), sizeof(uint8_t), compressedSize);
	
	_fileSystem->close(stream);
}
