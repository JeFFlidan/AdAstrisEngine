#pragma once
#include <glm/vec4.hpp>

namespace actors
{	
	struct PointLight
	{
		glm::vec4 colorAndIntensity;	// Intensity in lm
		glm::vec4 positionAndAttRadius;
		float sourceRadius;
	};

	struct SpotLight
	{
		glm::vec4 colorAndIntensity;	// Intensity in lm
		glm::vec4 positionAndDistance;
		glm::vec4 spotDirAndInnerConeRadius;
		float outerConeRadius;
	};

	struct DirectionLight
	{
		glm::vec4 direction;
		glm::vec4 colorAndIntensity;	// w = intensity
	};
}

