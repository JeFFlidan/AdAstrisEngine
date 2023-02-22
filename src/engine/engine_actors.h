#pragma once
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace ad_astris::actors
{	
	struct PointLight
	{
		glm::vec4 colorAndIntensity;	// Intensity in lm
		glm::vec4 positionAndAttRadius;
		float sourceRadius;
		float farPlane;	// I have to think about this field
		int isVisible{ 1 };
		int castShadows{ 1 };
		glm::mat4 lightSpaceMat[6];
	};

	struct SpotLight
	{
		glm::vec4 colorAndIntensity;	// Intensity in lm
		glm::vec4 positionAndDistance;
		glm::vec4 rotationAndInnerConeRadius;
		glm::mat4 lightSpaceMat;
		float outerConeRadius;
		int isVisible{ 1 };
		int castShadows{ 1 };
		float farPlane;
	};

	struct DirectionalLight
	{
		glm::vec4 direction;
		glm::vec4 colorAndIntensity;	// w = intensity
		glm::mat4 lightSpaceMat;
		int isVisible{ 0 };
		int castShadows{ 1 };
		int data1, data2;
	};
}

