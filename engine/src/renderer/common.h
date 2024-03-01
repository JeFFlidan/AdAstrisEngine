#pragma once

#include "ecs/entity_filter.h"
#include "engine_core/basic_components.h"

// GPU means that data is for shaders, CPU means that data shouldn't be sent to the GPU
namespace ad_astris::renderer::impl
{
	inline uint32_t FRAME_INDEX = 0;
	inline const ecs::EntityFilter<ecore::StaticObjectTag, ecore::OpaquePBRMaterialComponent> STATIC_OPAQUE_FILTER;
	inline uint32_t IMAGE_WIDTH = 1920;
	inline uint32_t IMAGE_HEIGHT = 1080;
}
