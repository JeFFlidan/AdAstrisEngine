#pragma once

#include <vk_types.h>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <mesh_asset.h>

struct VertexInputDescription
{
	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct RenderBounds
{
	glm::vec3 origin;
	float radius;
	glm::vec3 extents;
	bool valid;
};

struct Mesh
{
	std::vector<assets::Vertex_f32_PNCV> _vertices;
	std::vector<uint32_t> _indices;
	AllocatedBuffer _vertexBuffer;
	AllocatedBuffer _indexBuffer;
	RenderBounds _bounds;

	bool load_from_obj(const char* filename);
	bool load_from_mesh_asset(const char* assetPath);

	static VertexInputDescription get_vertex_description();
};
