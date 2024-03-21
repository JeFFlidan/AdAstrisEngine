#include "render_graph.h"
#include "renderer_resource_manager.h"
#include "pipeline_manager.h"
#include "shader_compiler/shader_manager.h"
#include "render_core/public/render_core_module.h"
#include "core/module_manager.h"
#include "core/global_objects.h"
#include <memory>

namespace ad_astris::rcore
{
	class RenderCoreModule : public IRenderCoreModule
	{
		public:
			virtual void startup_module(ModuleManager* moduleManager) override;
			virtual IShaderManager* get_shader_manager() override;
			virtual IRenderGraph* get_render_graph() override;
			virtual IRendererResourceManager* get_renderer_resource_manager() override;
			virtual IPipelineManager* get_pipeline_manager() override;

		private:
			std::unique_ptr<IRenderGraph> _renderGraph;
			std::unique_ptr<IShaderManager> _shaderManager;
			std::unique_ptr<IRendererResourceManager> _rendererResourceManager;
			std::unique_ptr<IPipelineManager> _pipelineManager;
	};

	void RenderCoreModule::startup_module(ModuleManager* moduleManager)
	{
		// TODO I think I have to create config file for engine where I will store info about
		// 3D API. And based on this info engine will create VulkanRHI or D3D12RHI.
		// However, I don't know how to pass config info to the startup_module method.
		// Maybe, I'll read the config file with engine setting more than one time if it is not
		// a big overhead. For now, I will use
		//IVulkanRHIModule* rhiModule = moduleManager->load_module<IVulkanRHIModule>("libvulkan_rhi.dll");		// TODO module associated names in config file

		//_shaderCompiler = std::make_unique<impl::ShaderCompiler>();
		
		_rendererResourceManager = std::make_unique<impl::RendererResourceManager>();
		_renderGraph = std::make_unique<impl::RenderGraph>(_rendererResourceManager.get());
		_shaderManager = std::make_unique<impl::ShaderManager>();
		_pipelineManager = std::make_unique<impl::PipelineManager>(_rendererResourceManager.get());
	}

	IShaderManager* RenderCoreModule::get_shader_manager()
	{
		return _shaderManager.get();
	}

	IRenderGraph* RenderCoreModule::get_render_graph()
	{
		return _renderGraph.get();
	}

	IRendererResourceManager* RenderCoreModule::get_renderer_resource_manager()
	{
		return _rendererResourceManager.get();
	}

	IPipelineManager* RenderCoreModule::get_pipeline_manager()
	{
		return _pipelineManager.get();
	}

	DECL_MODULE_FUNCS(RenderCore, RenderCoreModule)
}