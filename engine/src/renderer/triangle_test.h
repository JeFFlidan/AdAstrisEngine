#pragma once

#include "rhi/engine_rhi.h"
#include "file_system/file_system.h"
#include "engine/render_core_module.h"
#include "renderer/rendering_base.h"

namespace ad_astris::renderer::impl
{
	class TriangleTest : public rcore::IRenderPassExecutor, public RenderingBase
	{
		public:
			TriangleTest(RenderingInitContext& initContext, io::FileSystem* fileSystem, rcore::IShaderManager* shaderManager);
			virtual void prepare_render_pass(rcore::IRenderGraph* renderGraph, rcore::IRendererResourceManager* rendererResourceManager) override;
			virtual void execute(rhi::CommandBuffer* cmd) override;
			rhi::RenderPass get_render_pass()
			{
				return _renderPass;
			}

			rhi::TextureView* get_texture_view() { return &_outputTextureView; }

		private:
			rhi::Pipeline _pipeline;
			rhi::RenderPass _renderPass;
			rhi::Texture _outputTexture;
			rhi::TextureView _outputTextureView;
	};
}