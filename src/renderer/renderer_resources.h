#pragma once

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

// GPU means that data is for shaders, CPU means that data shouldn't be sent to the GPU
namespace ad_astris::renderer
{
	struct GPUCameraData
	{
		glm::mat4 view;
		glm::mat4 oldView;
		glm::mat4 proj;
		glm::mat4 viewproj;
		glm::mat4 invViewProj;
		glm::vec4 cameraPosition;
	};

	struct GPUSceneData
	{
		uint32_t dirLightsAmount;
		uint32_t pointLightsAmount;
		uint32_t spotLightsAmount;
	};

	struct GPUSettings
	{
		glm::ivec2 viewportRes;
		uint32_t totalFrames;
		uint32_t isTaaEnabled{ 1 };
		float taaAlpha{ 8.0f };
		uint32_t data1, data2, data3;
	};

	struct GPUCullData
	{
		glm::mat4 view;
		float P00, P11, znear, zfar; // symmetric projection parameters
		float frustum[4];  // data for left/right/top/bottom frustum planes
		float lodBase, lodStep;  // lod distance i = base * pow(step, i)
		float pyramidWidth, pyramidHeight;  // depth pyramid size in texels

		uint32_t drawCount;

		int cullingEnabled;
		int lodEnabled;
		int occlusionEnabled;
		int distCull;
		int AABBcheck;
		float aabbmin_x;
		float aabbmin_y;
		float aabbmin_z;
		float aabbmax_x;
		float aabbmax_y;
		float aabbmax_z;	
	};

	struct GPUIndirectObject
	{
		VkDrawIndexedIndirectCommand command;
		uint32_t objectID;
		uint32_t batchID;
	};

	struct GPUInstance
	{
		uint32_t objectID;
		uint32_t batchID;
	};

	struct CPUCullParams
	{
		bool occlusionCull;
		bool frustumCull;
		float drawDist;
		bool aabb;
		glm::vec3 aabbmin;
		glm::vec3 aabbmax;
		glm::mat4 projMatrix;
		glm::mat4 viewMatrix;
	};
}