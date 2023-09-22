#pragma once

#include "rhi/engine_rhi.h"
#include "file_system/file_system.h"
#include "engine/render_core_module.h"

namespace ad_astris::renderer::impl
{
	class TriangleTest
	{
		public:
			TriangleTest(rhi::IEngineRHI* rhi, io::FileSystem* fileSystem, rcore::IShaderManager* shaderManager, uint32_t width, uint32_t height);
			void draw(rhi::CommandBuffer cmdBuffer);
			rhi::RenderPass get_render_pass()
			{
				return _renderPass;
			}

			rhi::TextureView* get_texture_view() { return &_outputTextureView; }

		private:
			rhi::IEngineRHI* _rhi;
			rhi::Pipeline _pipeline;
			rhi::RenderPass _renderPass;
			rhi::Texture _outputTexture;
			rhi::TextureView _outputTextureView;
	};
}