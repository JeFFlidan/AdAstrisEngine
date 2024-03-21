#pragma once

#include "../rendering_base.h"

namespace ad_astris::renderer::impl
{
	class Culling : public rcore::IRenderPassExecutor, public RenderingBase
	{
		public:
			Culling(RenderingInitContext& initContext) : RenderingBase(initContext) { }
			virtual void prepare_render_pass() override;
			virtual void execute(rhi::CommandBuffer* cmd) override;
	};
}