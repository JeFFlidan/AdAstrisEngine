#include "renderer.h"

#include "render_pass_context.h"
#include "engine/vulkan_rhi_module.h"
#include "application/editor_module.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void Renderer::init(RendererInitializationContext& rendererInitContext)
{
	_resourceManager = rendererInitContext.resourceManager;
	_rendererSubsettings = rendererInitContext.projectSettings->get_subsettings<ecore::RendererSubsettings>();
	_taskComposer = rendererInitContext.taskComposer;
	_eventManager = rendererInitContext.eventManager;
	_mainWindow = rendererInitContext.mainWindow;
	ModuleManager* moduleManager = rendererInitContext.moduleManager;

	switch (_rendererSubsettings->get_graphics_api())
	{
		case ecore::GraphicsAPI::VULKAN:
		{
			auto rhiModule = moduleManager->load_module<IVulkanRHIModule>("VulkanRHI");
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
	rhiInitContext.window = rendererInitContext.mainWindow;
	rhiInitContext.fileSystem = _resourceManager->get_file_system();
	rhi::SwapChainInfo swapChainInfo;
	swapChainInfo.width = _rendererSubsettings->get_render_area_width();
	swapChainInfo.height = _rendererSubsettings->get_render_area_height();
	swapChainInfo.sync = _rendererSubsettings->is_vsync_used();
	bool useTripleBuffering = _rendererSubsettings->is_triple_buffering_used();
	swapChainInfo.buffersCount = useTripleBuffering ? 3 : 2;
	rhiInitContext.swapChainInfo = &swapChainInfo;
	
	_rhi->init(rhiInitContext);

	//create_swap_chain();
	create_samplers();
	LOG_INFO("Renderer::init(): Loaded and initialized RHI module")

	auto rcoreModule = moduleManager->load_module<rcore::IRenderCoreModule>("RenderCore");
	_renderGraph = rcoreModule->get_render_graph();
	_shaderManager = rcoreModule->get_shader_manager();
	_rendererResourceManager = rcoreModule->get_renderer_resource_manager();
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
	shaderManagerInitContext.fileSystem = _resourceManager->get_file_system();
	shaderManagerInitContext.moduleManager = moduleManager;
	_shaderManager->init(shaderManagerInitContext);

	rcore::RendererResourceManagerInitContext rendererResourceManagerInitContext;
	rendererResourceManagerInitContext.rhi = _rhi;
	_rendererResourceManager->init(rendererResourceManagerInitContext);
	
	LOG_INFO("Renderer::init(): Loaded and initialized RenderCore module")
	
	MaterialManagerInitializationContext materialManagerInitContext;
	materialManagerInitContext.eventManager = _eventManager;
	materialManagerInitContext.resourceManager = _resourceManager;
	materialManagerInitContext.shaderManager = _shaderManager;
	materialManagerInitContext.taskComposer = _taskComposer;
	materialManagerInitContext.rhi = _rhi;
	_materialManager = std::make_unique<MaterialManager>(materialManagerInitContext);
	LOG_INFO("Renderer::init(): Initialized material system")

	SceneManagerInitializationContext sceneManagerInitContext;
	sceneManagerInitContext.rhi = _rhi;
	sceneManagerInitContext.eventManager = _eventManager;
	sceneManagerInitContext.taskComposer = _taskComposer;
	sceneManagerInitContext.resourceManager = _resourceManager;
	sceneManagerInitContext.rendererResourceManager = _rendererResourceManager;
	_sceneManager = std::make_unique<SceneManager>(sceneManagerInitContext);
	LOG_INFO("Renderer::init(): Initialized scene manager")

	_triangleTest = std::make_unique<TriangleTest>(_rhi, _resourceManager->get_file_system(), _shaderManager, _mainWindow->get_width(), _mainWindow->get_height());
	LOG_INFO("Renderer::init(): Initialized triangle test")

	rhi::UIWindowBackendInitContext uiBackendInitContext;
	uiBackendInitContext.rhi = _rhi;
	uiBackendInitContext.sampler = _samplers[SAMPLER_LINEAR_REPEAT];
	uiBackendInitContext.window = rendererInitContext.mainWindow;

	_uiWindowBackend->init(uiBackendInitContext);
	_uiWindowBackend->set_backbuffer(_triangleTest->get_texture_view(), &_samplers[SAMPLER_LINEAR_CLAMP]);
	_uiWindowBackend->get_callbacks(rendererInitContext.uiBackendCallbacks);
	LOG_INFO("Renderer::init(): Initialized ui backend")
}

void Renderer::cleanup()
{
	_renderGraph->cleanup();
	_uiWindowBackend->cleanup();
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
	uint32_t acquiredImageIndex;
	if (!_rhi->acquire_next_image(acquiredImageIndex, _frameIndex))
	{
		_uiWindowBackend->set_backbuffer(_triangleTest->get_texture_view(), &_samplers[SAMPLER_LINEAR_CLAMP]);
		return;
	}

	_sceneManager->setup_global_buffers();

	rhi::CommandBuffer cmd;
	_rhi->begin_command_buffer(&cmd);
	rhi::ClearValues clearValues;
	clearValues.color = { 0.1f, 0.1f, 0.1f, 1.0f };
	rhi::Viewport viewport;
	viewport.width = _mainWindow->get_width();
	viewport.height = _mainWindow->get_height();
	std::vector<rhi::Viewport> viewports = { viewport };
	_rhi->set_viewports(&cmd, viewports);

	rhi::Scissor scissor;
	scissor.right = _mainWindow->get_width();
	scissor.bottom = _mainWindow->get_height();
	std::vector<rhi::Scissor> scissors = { scissor };
	_rhi->set_scissors(&cmd, scissors);
	_triangleTest->draw(cmd);
	
	_rhi->begin_rendering_swap_chain(&cmd, &clearValues);
	if (_mainWindow->is_running())
		_uiWindowBackend->draw(&cmd);
	_rhi->end_rendering_swap_chain(&cmd);
	
	_rhi->submit(rhi::QueueType::GRAPHICS);
	if (!_rhi->present())
	{
		_uiWindowBackend->set_backbuffer(_triangleTest->get_texture_view(), &_samplers[SAMPLER_LINEAR_CLAMP]);
		_frameIndex = 0;
	}
	else
	{
		get_current_frame_index();
	}
}

void Renderer::get_current_frame_index()
{
	if (++_frameIndex > 2)
		_frameIndex = 0;
}

void Renderer::create_swap_chain()
{
	rhi::SwapChainInfo swapChainInfo;
	swapChainInfo.width = _rendererSubsettings->get_render_area_width();
	swapChainInfo.height = _rendererSubsettings->get_render_area_height();
	swapChainInfo.sync = _rendererSubsettings->is_vsync_used();
	bool useTripleBuffering = _rendererSubsettings->is_triple_buffering_used();
	swapChainInfo.buffersCount = useTripleBuffering ? 3 : 2;
	_rhi->create_swap_chain(&_swapChain, &swapChainInfo);
}

void Renderer::create_samplers()
{
	rhi::SamplerInfo samplerInfo;
	samplerInfo.filter = rhi::Filter::MIN_MAG_MIP_LINEAR;
	samplerInfo.addressMode = rhi::AddressMode::REPEAT;
	samplerInfo.borderColor = rhi::BorderColor::FLOAT_TRANSPARENT_BLACK;
	samplerInfo.maxAnisotropy = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = std::numeric_limits<float>::max();
	_rhi->create_sampler(&_samplers[SAMPLER_LINEAR_REPEAT], &samplerInfo);

	samplerInfo.addressMode = rhi::AddressMode::CLAMP_TO_EDGE;
	_rhi->create_sampler(&_samplers[SAMPLER_LINEAR_CLAMP], &samplerInfo);

	samplerInfo.addressMode = rhi::AddressMode::MIRRORED_REPEAT;
	_rhi->create_sampler(&_samplers[SAMPLER_LINEAR_MIRROR], &samplerInfo);

	samplerInfo.filter = rhi::Filter::MIN_MAG_MIP_NEAREST;
	samplerInfo.addressMode = rhi::AddressMode::REPEAT;
	_rhi->create_sampler(&_samplers[SAMPLER_NEAREST_CLAMP], &samplerInfo);

	samplerInfo.addressMode = rhi::AddressMode::CLAMP_TO_EDGE;
	_rhi->create_sampler(&_samplers[SAMPLER_NEAREST_CLAMP], &samplerInfo);

	samplerInfo.addressMode = rhi::AddressMode::MIRRORED_REPEAT;
	_rhi->create_sampler(&_samplers[SAMPLER_NEAREST_MIRROR], &samplerInfo);
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
