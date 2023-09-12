#pragma once

#include "rhi/engine_rhi.h"
#include "file_system/file_system.h"

namespace ad_astris::renderer::impl
{
	class TriangleTest
	{
		public:
			TriangleTest(rhi::IEngineRHI* rhi, io::FileSystem* fileSystem);
			void draw();

		private:
			rhi::IEngineRHI* _rhi;
			rhi::Pipeline _pipeline;
			rhi::RenderPass _renderPass;
	};
}