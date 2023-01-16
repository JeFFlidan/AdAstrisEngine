#pragma once
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace actors
{	
	struct PointLight
	{
		glm::vec4 colorAndIntensity;	// Intensity in lm
		glm::vec4 positionAndAttRadius;
		float sourceRadius;
		float farPlane;	// I have to think about this field
		float data1, data2;
		glm::mat4 lightSpaceMat[6];
	};

	struct SpotLight
	{
		glm::vec4 colorAndIntensity;	// Intensity in lm
		glm::vec4 positionAndDistance;
		glm::vec4 spotDirAndInnerConeRadius;
		glm::mat4 lightSpaceMat;
		float outerConeRadius;
		float nearPlane;
		float farPlane;
		float data1 = 0.0;
	};

	struct DirectionLight
	{
		glm::vec4 direction;
		glm::vec4 colorAndIntensity;	// w = intensity
		glm::mat4 lightSpaceMat;
	};
}

