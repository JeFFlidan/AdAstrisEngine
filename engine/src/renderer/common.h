#pragma once

#include <glm/glm/mat4x4.hpp>
#include <glm/glm/vec4.hpp>
#include <glm/glm/vec2.hpp>

// GPU means that data is for shaders, CPU means that data shouldn't be sent to the GPU
namespace ad_astris::renderer::impl
{
	struct GPUCameraData
	{
		XMFLOAT4X4 view;
		XMFLOAT4X4 oldView;
		XMFLOAT4X4 proj;
		XMFLOAT4X4 viewproj;
		XMFLOAT4X4 invViewProj;
		XMFLOAT4 cameraPosition;
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
		XMFLOAT4X4 view;
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

	// struct GPUIndirectObject
	// {
	// 	VkDrawIndexedIndirectCommand command;
	// 	uint32_t objectID;
	// 	uint32_t batchID;
	// };

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
		XMFLOAT3 aabbmin;
		XMFLOAT3 aabbmax;
		XMFLOAT4X4 projMatrix;
		XMFLOAT4X4 viewMatrix;
	};

	struct PointLight
	{
		XMFLOAT4 color;
		XMFLOAT4 locationAndAttenuationRadius;
		XMFLOAT4X4 lightSpaceMat[6];
		float intensity;		// In lm
		bool castShadows;
		bool isVisible;
		bool isWorldAffected;
		bool isTemperatureUsed;
		float temperature;
		float farPlane;
		int32_t emptyPlace1{ 0 };
	};

	struct DirectionalLight
	{
		XMFLOAT4 color;
		XMFLOAT4 direction;
		XMFLOAT4X4 lightSpaceMat;
		float intensity;		// In candelas
		bool isVisible;
		bool castShadows;
		bool isWorldAffected;
		bool isTemperatureUsed;
		float temperature;
		int32_t emptyPlace1{ 0 }, emptyPlace2{ 0 };
	};

	struct SpotLight
	{
		XMFLOAT4 color;
		XMFLOAT4 locationAndDistance;
		XMFLOAT4 rotationAndInnerConeRadius;
		XMFLOAT4X4 lightSpaceMat;
		float intensity;		// In lm
		float outerConeRadius;
		bool castShadows;
		bool isVisible;
		bool isWorldAffected;
		bool isTemperatureUsed;
		float temperature;
		float farPlane;
	};
}
