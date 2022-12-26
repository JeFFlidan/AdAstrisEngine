#pragma once
#include <glm/vec4.hpp>

namespace actors
{	
	struct PointLight
	{
		glm::vec4 color;
		glm::vec4 position;
		float attenuationRadius;
		float sourceRadius;
		float intensity;
	};

	struct SpotLight
	{
		glm::vec4 color;
		glm::vec4 position;
		float innerConeRadius;
		float outerConeRadius;
		float distance;
		float intensity;
	};

	struct DirectionLight
	{
		glm::vec4 direction;
		glm::vec4 colorAndIntensity;	// w = intensity
	};
}

