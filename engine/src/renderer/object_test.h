#pragma once

#include "rendering_base.h"
#include "engine/render_core_module.h"

namespace ad_astris::renderer::impl
{
	class ObjectTest : public rcore::IRenderPassExecutor, public RenderingBase
	{
		public:
			ObjectTest(RenderingInitContext& initContext, io::FileSystem* fileSystem, rcore::IShaderManager* shaderManager);
			virtual void prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager) override;
			virtual void execute(rhi::CommandBuffer* cmd) override;

		private:
			rhi::Pipeline _pipeline;
	};
}