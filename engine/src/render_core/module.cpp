#include "render_graph.h"
#include "shader_compiler.h"
#include "engine/render_core_module.h"
#include "engine/vulkan_rhi_module.h"
#include "rhi/engine_rhi.h"
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
			IRenderGraph* _renderGraph;
			IShaderCompiler* _shaderCompiler;
	};

	void RenderCoreModule::startup_module(ModuleManager* moduleManager)
	{
		// TODO I think I have to create config file for engine where I will store info about
		// 3D API. And based on this info engine will create VulkanRHI or D3D12RHI.
		// However, I don't know how to pass config info to the startup_module method.
		// Maybe, I'll read the config file with engine setting more than one time if it is not
		// a big overhead. For now, I will use
		IVulkanRHIModule* rhiModule = moduleManager->load_module<IVulkanRHIModule>("libvulkan_rhi.dll");		// TODO module associated names in config file

		_shaderCompiler = new impl::ShaderCompiler();
		
		rhi::IEngineRHI* rhi = rhiModule->create_vulkan_rhi();
		_renderGraph = new impl::RenderGraph(rhi);
	}

	IShaderCompiler* RenderCoreModule::get_shader_compiler()
	{
		return _shaderCompiler;
	}

	IRenderGraph* RenderCoreModule::get_render_graph()
	{
		return _renderGraph;
	}

	extern "C" RENDER_CORE_API IRenderCoreModule* register_module()
	{
		return new RenderCoreModule();
	}
}