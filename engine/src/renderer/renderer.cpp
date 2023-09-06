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
	_engineObjectsCreator = initializationContext.engineObjectsCreator;
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

	rhi::SwapChainInfo swapChainInfo;
	swapChainInfo.width = _rendererSubsettings->get_render_area_width();
	swapChainInfo.height = _rendererSubsettings->get_render_area_height();
	swapChainInfo.sync = _rendererSubsettings->is_vsync_used();
	bool useTripleBuffering = _rendererSubsettings->is_triple_buffering_used();
	swapChainInfo.buffersCount = useTripleBuffering ? 3 : 2;
	rhi::SwapChain swapChain;
	_rhi->create_swap_chain(&swapChain, &swapChainInfo);
	LOG_INFO("Renderer::init(): Created swap chain")
	
	MaterialManagerInitializationContext materialManagerInitContext;
	materialManagerInitContext.eventManager = _eventManager;
	materialManagerInitContext.resourceManager = _resourceManager;
	materialManagerInitContext.shaderCompiler = _shaderCompiler;
	materialManagerInitContext.taskComposer = _taskComposer;
	materialManagerInitContext.rhi = _rhi;
	_materialManager = std::make_unique<MaterialManager>(materialManagerInitContext);
	LOG_INFO("Renderer::init(): Initialized material system")

	SceneManagerInitializationContext sceneManagerInitContext;
	sceneManagerInitContext.rhi = _rhi;
	sceneManagerInitContext.eventManager = _eventManager;
	sceneManagerInitContext.taskComposer = _taskComposer;
	_sceneManager = std::make_unique<SceneManager>(sceneManagerInitContext);
	LOG_INFO("Renderer::init(): Initialized scene manager")
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
	LOG_INFO("AFTER EXECUTING TRANSFER OPERATIONS")
}

void Renderer::draw()
{
	uint32_t currentFrameIndex = get_current_frame_index();
	uint32_t acquiredImageIndex;
	_rhi->acquire_next_image(acquiredImageIndex, currentFrameIndex);
	_sceneManager->setup_global_buffers();
	_sceneManager->execute_transfer_operations();
	test_light_submanager();
	++_frameNumber;
}

uint32_t Renderer::get_current_frame_index()
{
	uint32_t bufferCount = _rendererSubsettings->is_triple_buffering_used() ? 3 : 2;
	return _frameNumber % bufferCount;
}

void Renderer::test_light_submanager()
{
	if (_wasLightSubmanagerTested)
		return;
	ecore::EditorObjectCreationContext creationContext;
	creationContext.location = glm::vec3(1.0f);
	for (uint32_t i = 0; i != 70; ++i)
	{
		_engineObjectsCreator->create_point_light(creationContext);
		_engineObjectsCreator->create_spot_light(creationContext);
	}

	for (uint32_t i = 0; i != 30; ++i)
		_engineObjectsCreator->create_directional_light(creationContext);
	_wasLightSubmanagerTested = true;
}
