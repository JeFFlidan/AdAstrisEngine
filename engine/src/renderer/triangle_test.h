#pragma once

#include "rhi/engine_rhi.h"
#include "file_system/file_system.h"
#include "engine/render_core_module.h"

namespace ad_astris::renderer::impl
{
	class TriangleTest
	{
		public:
			TriangleTest(rhi::IEngineRHI* rhi, io::FileSystem* fileSystem, rcore::IShaderManager* shaderManager);
			void draw();

		private:
			rhi::IEngineRHI* _rhi;
			rhi::Pipeline _pipeline;
			rhi::RenderPass _renderPass;
	};
}