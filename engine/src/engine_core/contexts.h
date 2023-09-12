#pragma once

#include "uuid.h"
#include <glm/glm/vec3.hpp>

namespace ad_astris::ecore
{
	struct EditorObjectCreationContext
	{
		UUID uuid;
		glm::vec3 location;
	};
}