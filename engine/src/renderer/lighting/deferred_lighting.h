#pragma once

#include "renderer/rendering_base.h"

namespace ad_astris::renderer::impl
{
	class GBuffer : public rcore::IRenderPassExecutor, public RenderingBase
	{
		public:
			GBuffer(RenderingInitContext& initContext) : RenderingBase(initContext) { }
			virtual void prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager) override;
			virtual void execute(rhi::CommandBuffer* cmd) override;
	};

	class DeferredLighting : public rcore::IRenderPassExecutor, public RenderingBase
	{
		public:
			DeferredLighting(RenderingInitContext& initContext) : RenderingBase(initContext) { }
			virtual void prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager) override;
			virtual void execute(rhi::CommandBuffer* cmd) override;
	};
}