#include "renderer.h"

#include "engine/vulkan_rhi_module.h"
#include "application/editor_module.h"
#include "lighting/deferred_lighting.h"
#include "transparency/oit.h"
#include "postprocessing/temporal_filter.h"
#include "swap_chain_pass.h"
#include "shader_interop_renderer.h"
#include "object_test.h"

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
	_world = rendererInitContext.world;
	ModuleManager* moduleManager = rendererInitContext.moduleManager;
	ecs::set_type_info_table(rendererInitContext.typeInfoTable);

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

	SceneManagerInitializationContext sceneManagerInitContext;
	sceneManagerInitContext.rhi = _rhi;
	sceneManagerInitContext.eventManager = _eventManager;
	sceneManagerInitContext.taskComposer = _taskComposer;
	sceneManagerInitContext.resourceManager = _resourceManager;
	sceneManagerInitContext.rendererResourceManager = _rendererResourceManager;
	sceneManagerInitContext.world = _world;
	_sceneManager = std::make_unique<SceneManager>(sceneManagerInitContext);
	LOG_INFO("Renderer::init(): Initialized scene manager")

	rhi::UIWindowBackendInitContext uiBackendInitContext;
	uiBackendInitContext.rhi = _rhi;
	uiBackendInitContext.sampler = _samplers[SAMPLER_LINEAR_REPEAT];
	uiBackendInitContext.window = rendererInitContext.mainWindow;

	_uiWindowBackend->init(uiBackendInitContext);
	_uiWindowBackend->get_callbacks(rendererInitContext.uiBackendCallbacks);
	LOG_INFO("Renderer::init(): Initialized ui backend")
	
	create_uniform_buffers();
	create_samplers();
	LOG_INFO("Renderer::init(): Created renderer resources")
}

void Renderer::cleanup()
{
	_renderGraph->cleanup();
	_uiWindowBackend->cleanup();
	_rhi->cleanup();
}

void Renderer::bake()
{
	LOG_INFO("Renderer::bake(): Start baking")

	RenderingInitContext renderingInitContext;
	renderingInitContext.rhi = _rhi;
	renderingInitContext.mainWindow = _mainWindow;
	renderingInitContext.sceneManager = _sceneManager.get();
	//
	// _renderPassExecutors.emplace_back(new GBuffer(renderingInitContext));
	// LOG_INFO("AFTER EMPLACING GBUFFER")
	// _renderPassExecutors.emplace_back(new DeferredLighting(renderingInitContext));
	// LOG_INFO("AFTER EMPLACING DEFERRED")
	// _renderPassExecutors.emplace_back(new TemporalFilter(renderingInitContext));
	// _renderPassExecutors.emplace_back(new OITGeometry(renderingInitContext));
	// _renderPassExecutors.emplace_back(new OIT(renderingInitContext));

	_renderPassExecutors.emplace_back(new ObjectTest(renderingInitContext, _resourceManager->get_file_system(), _shaderManager));

	for (auto& executor : _renderPassExecutors)
		executor->prepare_render_pass(_renderGraph, _rendererResourceManager);

	auto swapChainPass = std::make_unique<SwapChainPass>(renderingInitContext, _uiWindowBackend);
	swapChainPass->prepare_render_pass(_renderGraph, _rendererResourceManager);
	_renderGraph->set_swap_chain_executor(swapChainPass.get());
	_renderPassExecutors.push_back(std::move(swapChainPass));

	_renderGraph->set_swap_chain_input("ObjectOutput");
	LOG_INFO("BEFORE BAKE")
	_renderGraph->bake();
	LOG_INFO("AFTER BAKE")

	_renderGraph->log();
	
	_uiWindowBackend->set_backbuffer(_rendererResourceManager->get_texture_view("ObjectOutput"), &_samplers[SAMPLER_LINEAR_CLAMP]);
	LOG_INFO("Renderer::bake(): Finished baking")
}

void Renderer::draw(DrawContext& drawContext)
{
	uint32_t acquiredImageIndex;
	if (!_rhi->acquire_next_image(acquiredImageIndex, _frameIndex))
	{
		_uiWindowBackend->set_backbuffer(_rendererResourceManager->get_texture_view("ObjectOutput"), &_samplers[SAMPLER_LINEAR_CLAMP]);
		return;
	}

	_sceneManager->setup_global_buffers();
	setup_cameras(drawContext);
	setup_frame_data(drawContext);

	tasks::TaskGroup taskGroup;
	_renderGraph->draw(&taskGroup);
	
	_rhi->submit(rhi::QueueType::GRAPHICS);
	if (!_rhi->present())
	{
		_uiWindowBackend->set_backbuffer(_rendererResourceManager->get_texture_view("ObjectOutput"), &_samplers[SAMPLER_LINEAR_CLAMP]);
		_frameIndex = 0;
	}
	else
	{
		get_current_frame_index();
	}
}

void Renderer::get_current_frame_index()
{
	if (++_frameIndex > _rhi->get_buffer_count() - 1)
		_frameIndex = 0;
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

void Renderer::create_uniform_buffers()
{
	rhi::BufferInfo bufferInfo;
	bufferInfo.size = sizeof(RendererCamera) * 16;
	bufferInfo.bufferUsage = rhi::ResourceUsage::UNIFORM_BUFFER | rhi::ResourceUsage::TRANSFER_DST;
	bufferInfo.memoryUsage = rhi::MemoryUsage::CPU_TO_GPU;
	for (uint32_t i = 0; i != _rhi->get_buffer_count(); ++i)
		_rendererResourceManager->allocate_buffer("CameraUB" + std::to_string(i), bufferInfo);

	bufferInfo.size = sizeof(FrameUB);
	for (uint32_t i = 0; i != _rhi->get_buffer_count(); ++i)
		_rendererResourceManager->allocate_buffer("FrameUB" + std::to_string(i), bufferInfo);
}

void Renderer::setup_cameras(DrawContext& preDrawContext)
{
	auto cameraComponent = _world->get_entity_manager()->get_component<ecore::CameraComponent>(preDrawContext.activeCamera);
	auto transformComponent = _world->get_entity_manager()->get_component_const<ecore::TransformComponent>(preDrawContext.activeCamera);
	RendererCamera rendererCamera[16];
	rendererCamera[0].location = transformComponent->location;
	rendererCamera[0].view = cameraComponent->view;
	rendererCamera[0].projection = cameraComponent->projection;
	rendererCamera[0].viewProjection = cameraComponent->viewProjection;
	rendererCamera[0].inverseView = cameraComponent->inverseView;
	rendererCamera[0].inverseProjection = cameraComponent->inverseProjection;
	rendererCamera[0].inverseViewProjection = cameraComponent->inverseViewProjection;
	rendererCamera[0].zNear = cameraComponent->zNear;
	rendererCamera[0].zFar = cameraComponent->zFar;
	rendererCamera[0].up = cameraComponent->up;
	rhi::Buffer* cameraUB = _rendererResourceManager->get_buffer("CameraUB" + std::to_string(_frameIndex));
	_rhi->update_buffer_data(cameraUB, sizeof(RendererCamera) * 16, rendererCamera);
	_rhi->bind_uniform_buffer(cameraUB, UB_CAMERA_SLOT);
}

void Renderer::setup_frame_data(DrawContext& preDrawContext)
{
	FrameUB frameData;
	rhi::Buffer* modelInstanceBuffer = _sceneManager->get_model_instance_storage_buffer();
	frameData.modelInstanceBufferIndex = _rhi->get_descriptor_index(modelInstanceBuffer);
	rhi::Buffer* frameUB = _rendererResourceManager->get_buffer("FrameUB" + std::to_string(_frameIndex));
	_rhi->update_buffer_data(frameUB, sizeof(FrameUB), &frameData);
	_rhi->bind_uniform_buffer(frameUB, UB_FRAME_SLOT);
}
