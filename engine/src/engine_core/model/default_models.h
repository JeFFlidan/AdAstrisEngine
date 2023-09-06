#pragma once

#include "model_common.h"

namespace ad_astris::ecore
{
	struct Plane
	{
		std::vector<model::VertexF32PC> vertices = {
			{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
			{ { -1.0f, -1.0f,0.0f }, { 0.0f, 0.0f } },
			{ { 1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
			{ { 1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
			{ {  1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
			{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } }
		};
	};
}