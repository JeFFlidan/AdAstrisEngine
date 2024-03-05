#pragma once

#include "scene_manager/culling_submanager.h"
#include <cstdint>

namespace ad_astris::renderer::impl
{
	class Utils
	{
		public:
			static uint32_t get_group_count(uint32_t threadCount, uint32_t localSize)
			{
				return (threadCount + localSize - 1) / localSize;
			}

			template<typename ...ARGS>
			static DepthPyramid& get_depth_pyramid(const ecs::EntityFilter<ARGS...>& filter, ecore::CameraIndex cameraIndex)
			{
				return *SCENE_MANAGER()->get_culling_submanager()->get_scene_indirect_buffers(filter, cameraIndex).depthPyramid;
			}

	};
}