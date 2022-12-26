#include <iostream>
#include <string>

#include <tiny_obj_loader.h>

#include <vk_mesh.h>

VertexInputDescription Mesh::get_vertex_description()
{
	VertexInputDescription description;
	
	VkVertexInputBindingDescription mainBinding{};
	mainBinding.binding = 0;
	mainBinding.stride = sizeof(assets::Vertex_f32_PNCV);
	mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	description.bindings.push_back(mainBinding);

	VkVertexInputAttributeDescription positionAttribute{};
	positionAttribute.binding = 0;
	positionAttribute.location = 0;
	positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	positionAttribute.offset = offsetof(assets::Vertex_f32_PNCV, position);

	VkVertexInputAttributeDescription normalAttribute{};
	normalAttribute.binding = 0;
	normalAttribute.location = 1;
	normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	normalAttribute.offset = offsetof(assets::Vertex_f32_PNCV, normal);

	VkVertexInputAttributeDescription colorAttribute{};
	colorAttribute.binding = 0;
	colorAttribute.location = 2;
	colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	colorAttribute.offset = offsetof(assets::Vertex_f32_PNCV, tangent);

	VkVertexInputAttributeDescription uvAttribute{};
	uvAttribute.binding = 0;
	uvAttribute.location = 3;
	uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
	uvAttribute.offset = offsetof(assets::Vertex_f32_PNCV, uv);

	description.attributes.push_back(positionAttribute);
	description.attributes.push_back(normalAttribute);
	description.attributes.push_back(colorAttribute);
	description.attributes.push_back(uvAttribute);

	return description;
}

bool Mesh::load_from_mesh_asset(const char* assetPath)
{
	assets::AssetFile file;
	bool loaded = assets::load_binaryFile(assetPath, file);

	if (!loaded)
	{
		std::cout << "Error when loading image\n";
		return false;
	}

	assets::MeshInfo meshInfo = assets::read_mesh_info(&file);

	assets::MeshBounds meshBounds = meshInfo.bounds;
	_bounds.origin = glm::vec3(meshBounds.origin[0], meshBounds.origin[1], meshBounds.origin[2]);
	_bounds.extents = glm::vec3(meshBounds.extents[0], meshBounds.extents[1], meshBounds.extents[2]);
	_bounds.radius = meshBounds.radius;
	_bounds.valid = true;
	
	_vertices.resize(meshInfo.vertexBufferSize / sizeof(assets::Vertex_f32_PNCV));
	_indices.resize(meshInfo.indexBufferSize / sizeof(uint32_t));
	assets::unpack_mesh(&meshInfo, file.binaryBlob.data(), file.binaryBlob.size(), (char*)_vertices.data(), (char*)_indices.data());

	return true;
}
