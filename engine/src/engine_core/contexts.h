#pragma once

#include "uuid.h"
#include "core/math_base.h"

namespace ad_astris::ecore
{
	struct EditorObjectCreationContext
	{
		UUID uuid;
		XMFLOAT3 location{0.0f, 0.0f, 0.0f};
		XMFLOAT4 rotation{0.0f, 0.0f, 0.0f, 1.0f};
		XMFLOAT3 scale{1.0f, 1.0f, 1.0f};
		UUID materialUUID;
	};
}