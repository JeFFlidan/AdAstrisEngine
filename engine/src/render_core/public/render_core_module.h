#pragma once

#include "shader_tools.h"
#include "renderer_resource_manager.h"
#include "render_graph.h"
#include "render_graph_types.h"
#include "pipeline_manager.h"

namespace ad_astris::rcore
{
	class IRenderCoreModule : public IModule
	{
		public:
			virtual IRenderGraph* get_render_graph() = 0;
			virtual IShaderManager* get_shader_manager() = 0;
			virtual IRendererResourceManager* get_renderer_resource_manager() = 0;
			virtual IPipelineManager* get_pipeline_manager() = 0;
	};
}