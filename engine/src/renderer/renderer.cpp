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

	rhi::SwapChainInfo swapChainInfo;
	swapChainInfo.width = _rendererSubsettings->get_render_area_width();
	swapChainInfo.height = _rendererSubsettings->get_render_area_height();
	swapChainInfo.sync = _rendererSubsettings->is_vsync_used();
	bool useTripleBuffering = _rendererSubsettings->is_triple_buffering_used();
	swapChainInfo.buffersCount = useTripleBuffering ? 3 : 2;
	_rhi->create_swap_chain(&_swapChain, &swapChainInfo);
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
	sceneManagerInitContext.resourceManager = _resourceManager;
	_sceneManager = std::make_unique<SceneManager>(sceneManagerInitContext);
	LOG_INFO("Renderer::init(): Initialized scene manager")

	_triangleTest = std::make_unique<TriangleTest>(_rhi, _resourceManager->get_file_system());
	LOG_INFO("Renderer::init(): Initialized triangle test")
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
	//test_rhi();
	LOG_INFO("AFTER CREATING SAMPLER")
}

void Renderer::draw()
{
	uint32_t currentFrameIndex = get_current_frame_index();
	uint32_t acquiredImageIndex;
	_rhi->acquire_next_image(acquiredImageIndex, currentFrameIndex);
	_sceneManager->setup_global_buffers();
	_triangleTest->draw();
	_rhi->submit(rhi::QueueType::GRAPHICS);
	_rhi->present();
	++_frameNumber;
}

uint32_t Renderer::get_current_frame_index()
{
	return _frameNumber % _swapChain.info.buffersCount;
}

void Renderer::test_rhi()
{
	rhi::SamplerInfo samplerInfo;
	samplerInfo.filter = rhi::Filter::MIN_MAG_MIP_LINEAR;
	samplerInfo.addressMode = rhi::AddressMode::REPEAT;
	samplerInfo.borderColor = rhi::BorderColor::UNDEFINED;
	rhi::Sampler sampler;
	LOG_INFO("BEFORE CREATING SAMPLER")
	_rhi->create_sampler(&sampler, &samplerInfo);
	LOG_INFO("AFTER CREATING SAMPLER. DESCRIPTOR INDEX: {}", _rhi->get_descriptor_index(&sampler));
}
