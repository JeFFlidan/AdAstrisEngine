#pragma once

#include "engine/render_core_module.h"
#include "render_pass.h"

namespace ad_astris::rcore::impl
{
	class PipelineManager : public IPipelineManager
	{
		public:
			PipelineManager(IRendererResourceManager* rendererResourceManager);
			virtual void init(PipelineManagerInitContext& initContext) override;
			virtual void bind_render_pass_to_pipeline(IRenderPass* renderPass, BuiltinPipelineType type) override;
			virtual void create_builtin_pipelines() override;
			virtual rhi::Pipeline* get_builtin_pipeline(BuiltinPipelineType pipelineType) override;

		private:
			IRendererResourceManager* _rendererResourceManager{ nullptr };
			rhi::IEngineRHI* _rhi{ nullptr };
			tasks::TaskComposer* _taskComposer{ nullptr };
			IShaderManager* _shaderManager{ nullptr };

			struct PipelineState
			{
				RenderPass* renderPass;
				rhi::Pipeline pipeline;
			};
		
			std::unordered_map<BuiltinPipelineType, PipelineState> _builtinPipelineStateByItsType;
			std::mutex _builtinPipelinesMutex;

			void load_builtin_shaders();
			void setup_default_graphics_pipeline_info(rhi::GraphicsPipelineInfo& pipelineInfo);
			void setup_formats(BuiltinPipelineType type, std::vector<rhi::Format>& colorAttachFormats, rhi::Format& depthAttachFormat);
			void create_gbuffer_pipeline();
			void create_deferred_lighting_pipeline();
			void create_occlusion_culling_pipeline();
	};
}