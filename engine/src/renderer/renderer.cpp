#include "renderer.h"
#include "application/editor_module.h"
#include "lighting/deferred_lighting.h"
#include "compute/occlusion_culling.h"
#include "transparency/oit.h"
#include "postprocessing/temporal_filter.h"
#include "swap_chain_pass.h"
#include "shader_interop_renderer.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void Renderer::init(RendererInitializationContext& rendererInitContext)
{
	_rendererSubsettings = rendererInitContext.projectSettings->get_subsettings<ecore::RendererSubsettings>();
	_mainWindow = rendererInitContext.mainWindow;
	
	init_global_objects(rendererInitContext.globalObjectContext);
	init_module_objects();
	
	UI_WINDOW_BACKEND()->get_callbacks(rendererInitContext.uiBackendCallbacks); // I must remove this bullshit
	LOG_INFO("Renderer::init(): Initialized UIBackendCallbacks")
	
	create_uniform_buffers();
	LOG_INFO("Renderer::init(): Created renderer resources")
}

void Renderer::cleanup()
{
	RENDER_GRAPH()->cleanup();
	UI_WINDOW_BACKEND()->cleanup();
	RHI()->cleanup();
	ModuleObjects::cleanup();
}

void Renderer::bake()
{
	LOG_INFO("Renderer::bake(): Start baking")

	RenderingInitContext renderingInitContext;
	renderingInitContext.rhi = RHI();
	renderingInitContext.mainWindow = _mainWindow;
	renderingInitContext.sceneManager = SCENE_MANAGER();
	renderingInitContext.pipelineManager = PIPELINE_MANAGER();
	
	_renderPassExecutors.emplace_back(new GBuffer(renderingInitContext));
	_renderPassExecutors.emplace_back(new DeferredLighting(renderingInitContext));
	
	for (auto& executor : _renderPassExecutors)
		executor->prepare_render_pass();

	auto swapChainPass = std::make_unique<SwapChainPass>(renderingInitContext);
	swapChainPass->prepare_render_pass();
	RENDER_GRAPH()->set_swap_chain_executor(swapChainPass.get());
	_renderPassExecutors.push_back(std::move(swapChainPass));

	RENDER_GRAPH()->set_swap_chain_input("DeferredLightingOutput");
	LOG_INFO("BEFORE BAKE")
	RENDER_GRAPH()->bake();
	LOG_INFO("AFTER BAKE")

	PIPELINE_MANAGER()->create_builtin_pipelines();

	RENDER_GRAPH()->log();
	
	set_backbuffer("DeferredLightingOutput");
	LOG_INFO("Renderer::bake(): Finished baking")
}

void Renderer::draw(DrawContext& drawContext)
{
	uint32_t acquiredImageIndex;
	if (!RHI()->acquire_next_image(acquiredImageIndex, _frameIndex))
	{
		set_backbuffer("DeferredLightingOutput");
		return;
	}
	profiler::Profiler::begin_gpu_frame();
	
	SCENE_MANAGER()->setup_global_buffers();
	setup_cameras(drawContext);
	setup_frame_data(drawContext);

	tasks::TaskGroup taskGroup;
	RENDER_GRAPH()->draw(&taskGroup);
	
	profiler::Profiler::end_gpu_frame();
	
	RHI()->submit(rhi::QueueType::GRAPHICS);
	if (!RHI()->present())
	{
		set_backbuffer("DeferredLightingOutput");
		_frameIndex = 0;
	}
	else
	{
		get_current_frame_index();
	}
}

void Renderer::init_global_objects(GlobalObjectContext* context)
{
	GlobalObjects::set_global_object_context(context);
	ecs::set_type_info_table(ECS_TYPE_INFO_TABLE());
	profiler::Profiler::init(PROFILER_INSTANCE());
	LOG_INFO("Renderer::init(): Initialized global objects")
}

void Renderer::init_module_objects()
{
	ModuleObjects::init_rhi_module(_rendererSubsettings, _mainWindow);
	LOG_INFO("Renderer::init(): Loaded and initialized RHI module")
	
	ModuleObjects::init_render_core_module(_rendererSubsettings);
	LOG_INFO("Renderer::init(): Loaded and initialized RenderCore module")
	
	ModuleObjects::init_scene_manager();
	LOG_INFO("Renderer::init(): Initialized scene manager")

	ModuleObjects::init_ui_window_backend(_mainWindow);
	LOG_INFO("Renderer::init(): Initialized ui backend")
}

void Renderer::get_current_frame_index()
{
	if (++_frameIndex > RHI()->get_buffer_count() - 1)
		_frameIndex = 0;
}

void Renderer::create_uniform_buffers()
{
	rhi::BufferInfo bufferInfo;
	bufferInfo.size = sizeof(RendererCamera) * 16;
	bufferInfo.bufferUsage = rhi::ResourceUsage::UNIFORM_BUFFER | rhi::ResourceUsage::TRANSFER_DST;
	bufferInfo.memoryUsage = rhi::MemoryUsage::CPU_TO_GPU;
	for (uint32_t i = 0; i != RHI()->get_buffer_count(); ++i)
		RENDERER_RESOURCE_MANAGER()->allocate_buffer("CameraUB" + std::to_string(i), bufferInfo);

	bufferInfo.size = sizeof(FrameUB);
	for (uint32_t i = 0; i != RHI()->get_buffer_count(); ++i)
		RENDERER_RESOURCE_MANAGER()->allocate_buffer("FrameUB" + std::to_string(i), bufferInfo);
}

void Renderer::setup_cameras(DrawContext& preDrawContext)
{
	auto cameraComponent = WORLD()->get_entity_manager()->get_component<ecore::CameraComponent>(preDrawContext.activeCamera);
	auto transformComponent = WORLD()->get_entity_manager()->get_component_const<ecore::TransformComponent>(preDrawContext.activeCamera);
	RendererCamera rendererCamera[16];
	rendererCamera[0].location = cameraComponent->eye;
	rendererCamera[0].view = cameraComponent->view;
	rendererCamera[0].projection = cameraComponent->projection;
	rendererCamera[0].viewProjection = cameraComponent->viewProjection;
	rendererCamera[0].inverseView = cameraComponent->inverseView;
	rendererCamera[0].inverseProjection = cameraComponent->inverseProjection;
	rendererCamera[0].inverseViewProjection = cameraComponent->inverseViewProjection;
	rendererCamera[0].zNear = cameraComponent->zNear;
	rendererCamera[0].zFar = cameraComponent->zFar;
	rendererCamera[0].up = cameraComponent->up;
	rhi::Buffer* cameraUB = RENDERER_RESOURCE_MANAGER()->get_buffer("CameraUB" + std::to_string(_frameIndex));
	RHI()->update_buffer_data(cameraUB, sizeof(RendererCamera) * 16, rendererCamera);
	RHI()->bind_uniform_buffer(cameraUB, UB_CAMERA_SLOT);
}

void Renderer::setup_frame_data(DrawContext& preDrawContext)
{
	FrameUB frameData;
	rhi::Buffer* modelInstanceBuffer = SCENE_MANAGER()->get_model_instance_storage_buffer();
	rhi::Buffer* materialBuffer = SCENE_MANAGER()->get_material_storage_buffer();
	rhi::Buffer* entityBuffer = SCENE_MANAGER()->get_entity_storage_buffer();
	rhi::Buffer* modelInstanceIDBuffer = SCENE_MANAGER()->get_indirect_buffer_desc()->get_model_instance_id_buffer();
	frameData.modelInstanceBufferIndex = RHI()->get_descriptor_index(modelInstanceBuffer);
	frameData.materialBufferIndex = RHI()->get_descriptor_index(materialBuffer);
	frameData.entityBufferIndex = RHI()->get_descriptor_index(entityBuffer);
	frameData.modelInstanceIDBufferIndex = RHI()->get_descriptor_index(modelInstanceIDBuffer);
	frameData.lightArrayOffset = 0;
	frameData.lightArrayCount = SCENE_MANAGER()->get_light_count();	// TODO Take info from scene manager
	rhi::Buffer* frameUB = RENDERER_RESOURCE_MANAGER()->get_buffer("FrameUB" + std::to_string(_frameIndex));
	RHI()->update_buffer_data(frameUB, sizeof(FrameUB), &frameData);
	RHI()->bind_uniform_buffer(frameUB, UB_FRAME_SLOT);
}

void Renderer::set_backbuffer(const std::string& textureName)
{
	UI_WINDOW_BACKEND()->set_backbuffer(RENDERER_RESOURCE_MANAGER()->get_texture_view(textureName), SCENE_MANAGER()->get_sampler(SAMPLER_LINEAR_CLAMP));
}
