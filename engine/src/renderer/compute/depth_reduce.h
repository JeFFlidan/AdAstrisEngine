#pragma once

#include "renderer/rendering_base.h"

namespace ad_astris::renderer::impl
{
	class DepthReduce : public rcore::IRenderPassExecutor, public RenderingBase
	{
		public:
			DepthReduce(RenderingInitContext& initContext) : RenderingBase(initContext) { }
			virtual void prepare_render_pass() override;
			virtual void execute(rhi::CommandBuffer* cmd) override;
	};
}