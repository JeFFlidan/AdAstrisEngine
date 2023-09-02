#include "renderer.h"

#include "render_pass_context.h"
#include "engine/vulkan_rhi_module.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void Renderer::init(RendererInitializationContext& initializationContext)
{
	_resourceManager = initializationContext.resourceManager;
	_rendererSubsettings = initializationContext.projectSettings->get_subsettings<ecore::RendererSubsettings>();
	_taskComposer = initializationContext.taskComposer;
	_eventManager = initializationContext.eventManager;
	ModuleManager* moduleManager = initializationContext.moduleManager;

	switch (_rendererSubsettings->get_graphics_api())
	{
		case ecore::GraphicsAPI::VULKAN:
		{
			_rhi = moduleManager->load_module<IVulkanRHIModule>("VulkanRHI")->create_vulkan_rhi();
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
	_rhi->init(initializationContext.mainWindow, _resourceManager->get_file_system());
	LOG_INFO("Renderer::init(): Loaded and initialized RHI module")

	auto rcoreModule = moduleManager->load_module<rcore::IRenderCoreModule>("RenderCore");
	_renderGraph = rcoreModule->get_render_graph();
	_shaderCompiler = rcoreModule->get_shader_compiler();
	_renderGraph->init(_rhi);
	_shaderCompiler->init(_resourceManager->get_file_system());
	LOG_INFO("Renderer::init(): Loaded and initialized RenderCore module")

	MaterialManagerInitializationContext materialManagerInitContext;
	materialManagerInitContext.eventManager = _eventManager;
	materialManagerInitContext.resourceManager = _resourceManager;
	materialManagerInitContext.shaderCompiler = _shaderCompiler;
	materialManagerInitContext.taskComposer = _taskComposer;
	materialManagerInitContext.rhi = _rhi;
	_materialManager = std::make_unique<MaterialManager>(materialManagerInitContext);
	LOG_INFO("Renderer::init(): Initialized material system")
}

void Renderer::cleanup()
{
	_renderGraph->cleanup();
	_rhi->cleanup();
}

void Renderer::bake()
{
	auto renderPassContext = std::make_unique<RenderPassContext>(_renderGraph);
	_materialManager->create_material_templates(renderPassContext.get());
	LOG_INFO("Renderer::bake(): Created renderer material templates")
}
