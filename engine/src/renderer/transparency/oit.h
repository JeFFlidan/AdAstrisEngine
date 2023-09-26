#pragma once

// Order independent transparency

#include "renderer/rendering_base.h"

namespace ad_astris::renderer::impl
{
	class OITGeometry : public rcore::IRenderPassExecutor, public RenderingBase
	{
		public:
			OITGeometry(RenderingInitContext& initContext) : RenderingBase(initContext) { }

			virtual void prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager) override;
			virtual void execute(rhi::CommandBuffer* cmd) override;
	};

	class OIT :  public rcore::IRenderPassExecutor, public RenderingBase
	{
		public:
			OIT(RenderingInitContext& initContext) : RenderingBase(initContext) { }

			virtual void prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager) override;
			virtual void execute(rhi::CommandBuffer* cmd) override;
	};
}
