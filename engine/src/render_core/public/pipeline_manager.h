#pragma once

#include "rhi/engine_rhi.h"
#include "core/global_objects.h"
#include "shader_tools.h"
#include "render_graph.h"

namespace ad_astris::rcore
{
	struct PipelineManagerInitContext
	{
		rhi::RHI* rhi{ nullptr };
		IShaderManager* shaderManager{ nullptr };
		tasks::TaskComposer* taskComposer{ nullptr };
	};

	enum class BuiltinPipelineType
	{
		GBUFFER = 0,
		DEFERRED_LIGHTING,
		OIT_GEOMETRY,
		OIT,
		CULLING,
		DEPTH_REDUCE
	};
	
	class IPipelineManager
	{
		public:
			~IPipelineManager() { }
			virtual void init(PipelineManagerInitContext& initContext) = 0;
			// If Vulkan is used, creates all builtin pipelines for dynamic rendering.
			virtual void bind_render_pass_to_pipeline(IRenderPass* renderPass, BuiltinPipelineType type) = 0;
			virtual void create_builtin_pipelines() = 0;
			virtual rhi::Pipeline* get_builtin_pipeline(BuiltinPipelineType pipelineType) = 0;
				
			// virtual rhi::Pipeline* create_custom_pipeline() = 0;
			// virtual rhi::Pipeline* get_custom_pipeline() = 0;
	};
	
}