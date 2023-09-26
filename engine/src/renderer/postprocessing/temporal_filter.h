#pragma once

#include "renderer/rendering_base.h"

namespace ad_astris::renderer::impl
{
	class TemporalFilter : public rcore::IRenderPassExecutor, public RenderingBase
	{
		public:
			TemporalFilter(RenderingInitContext& initContext) : RenderingBase(initContext) { }

			virtual void prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager) override;
			virtual void execute(rhi::CommandBuffer* cmd) override;
	};
}
