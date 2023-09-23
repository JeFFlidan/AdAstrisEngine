#include "render_graph.h"
#include "renderer_resource_manager.h"
#include "shader_compiler/shader_manager.h"
#include "engine/render_core_module.h"
#include "core/module_manager.h"
#include "profiler/logger.h"
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

		private:
			std::unique_ptr<IRenderGraph> _renderGraph;
			std::unique_ptr<IShaderManager> _shaderManager;
			std::unique_ptr<IRendererResourceManager> _rendererResourceManager;
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
		
		_renderGraph = std::make_unique<impl::RenderGraph>();
		_shaderManager = std::make_unique<impl::ShaderManager>();
		_rendererResourceManager = std::make_unique<impl::RendererResourceManager>();
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

	extern "C" RENDER_CORE_API IRenderCoreModule* register_module()
	{
		return new RenderCoreModule();
	}
}