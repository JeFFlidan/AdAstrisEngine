#include "module_objects.h"
#include "core/global_objects.h"
#include "vulkan_rhi/public/vulkan_rhi_module.h"
#include "scene_manager/scene_manager.h"

using namespace ad_astris;
using namespace renderer::impl;

void ModuleObjects::init_rhi_module(acore::IWindow* mainWindow)
{
	switch (_rendererSubsettings->get_graphics_api())
	{
		case ecore::GraphicsAPI::VULKAN:
		{
			auto rhiModule = MODULE_MANAGER()->load_module<vulkan::IVulkanRHIModule>("VulkanRHI");
			rhiModule->set_global_objects();
			_rhi = rhiModule->create_vulkan_rhi();
			_uiWindowBackend = rhiModule->get_ui_window_backend();
			break;
		}
		case ecore::GraphicsAPI::D3D12:
		{
			// TODO
			break;
		}
		case ecore::GraphicsAPI::UNDEFINED:
		{
			LOG_FATAL("Renderer::init(): Can't load any of the RHI modules because GraphicsAPI property = UNDEFINED")
			break;
		}
	}

	rhi::RHIInitContext rhiInitContext;
	rhiInitContext.window = mainWindow;
	rhiInitContext.validationMode = rhi::ValidationMode::ENABLED;
	rhi::SwapChainInfo swapChainInfo;
	swapChainInfo.width = _rendererSubsettings->get_render_area_width();
	swapChainInfo.height = _rendererSubsettings->get_render_area_height();
	swapChainInfo.vSync = _rendererSubsettings->is_vsync_used();
	bool useTripleBuffering = _rendererSubsettings->is_triple_buffering_used();
	swapChainInfo.buffersCount = useTripleBuffering ? 3 : 2;
	rhiInitContext.swapChainInfo = &swapChainInfo;
	
	_rhi->init(rhiInitContext);
}

void ModuleObjects::init_render_core_module()
{
	auto rcoreModule = MODULE_MANAGER()->load_module<rcore::IRenderCoreModule>("RenderCore");
	rcoreModule->set_global_objects();
	_renderGraph = rcoreModule->get_render_graph();
	_shaderManager = rcoreModule->get_shader_manager();
	_rendererResourceManager = rcoreModule->get_renderer_resource_manager();
	_pipelineManager = rcoreModule->get_pipeline_manager();
	_renderGraph->init(_rhi);

	rcore::ShaderManagerInitContext shaderManagerInitContext;
	shaderManagerInitContext.rhi = _rhi;
	switch (_rendererSubsettings->get_graphics_api())
	{
		case ecore::GraphicsAPI::VULKAN:
			shaderManagerInitContext.cacheType = rcore::ShaderCacheType::SPIRV;
		break;
		case ecore::GraphicsAPI::D3D12:
			shaderManagerInitContext.cacheType = rcore::ShaderCacheType::DXIL;
		break;
	}
	_shaderManager->init(shaderManagerInitContext);

	rcore::RendererResourceManagerInitContext rendererResourceManagerInitContext;
	rendererResourceManagerInitContext.rhi = _rhi;
	_rendererResourceManager->init(rendererResourceManagerInitContext);

	rcore::PipelineManagerInitContext pipelineManagerInitContext;
	pipelineManagerInitContext.rhi = _rhi;
	pipelineManagerInitContext.shaderManager = _shaderManager;
	pipelineManagerInitContext.taskComposer = TASK_COMPOSER();
	_pipelineManager->init(pipelineManagerInitContext);
}

void ModuleObjects::init_scene_manager()
{
	_sceneManager = new SceneManager();
}

void ModuleObjects::init_ui_window_backend(acore::IWindow* mainWindow)
{
	rhi::UIWindowBackendInitContext uiBackendInitContext;
	uiBackendInitContext.rhi = RHI();
	uiBackendInitContext.sampler = SCENE_MANAGER()->get_sampler(SAMPLER_LINEAR_CLAMP);
	uiBackendInitContext.window = mainWindow;
	uiBackendInitContext.fileSystem = RESOURCE_MANAGER()->get_file_system();
	_uiWindowBackend->init(uiBackendInitContext, SCENE_MANAGER()->get_sampler(SAMPLER_LINEAR_CLAMP));
}

void ModuleObjects::cleanup()
{
	delete _sceneManager;
}
