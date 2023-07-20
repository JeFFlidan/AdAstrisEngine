#include "render_graph.h"
#include "shader_compiler.h"
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
			virtual IShaderCompiler* get_shader_compiler() override;
			virtual IRenderGraph* get_render_graph() override;

		private:
			std::unique_ptr<IRenderGraph> _renderGraph;
			std::unique_ptr<IShaderCompiler> _shaderCompiler;
	};

	void RenderCoreModule::startup_module(ModuleManager* moduleManager)
	{
		// TODO I think I have to create config file for engine where I will store info about
		// 3D API. And based on this info engine will create VulkanRHI or D3D12RHI.
		// However, I don't know how to pass config info to the startup_module method.
		// Maybe, I'll read the config file with engine setting more than one time if it is not
		// a big overhead. For now, I will use
		//IVulkanRHIModule* rhiModule = moduleManager->load_module<IVulkanRHIModule>("libvulkan_rhi.dll");		// TODO module associated names in config file

		_shaderCompiler = std::make_unique<impl::ShaderCompiler>();
		
		_renderGraph = std::make_unique<impl::RenderGraph>();
	}

	IShaderCompiler* RenderCoreModule::get_shader_compiler()
	{
		return _shaderCompiler.get();
	}

	IRenderGraph* RenderCoreModule::get_render_graph()
	{
		return _renderGraph.get();
	}

	extern "C" RENDER_CORE_API IRenderCoreModule* register_module()
	{
		return new RenderCoreModule();
	}
}