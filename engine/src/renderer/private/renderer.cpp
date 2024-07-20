#include "renderer.h"
#include "common.h"
#include "application/editor_module.h"
#include "lighting/deferred_lighting.h"
#include "compute/culling.h"
#include "compute/depth_reduce.h"
#include "transparency/oit.h"
#include "postprocessing/temporal_filter.h"
#include "swap_chain_pass.h"
#include "shader_interop_renderer.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

void Renderer::init(RendererInitializationContext& rendererInitContext)
{
	ModuleObjects::set_renderer_subsettings(rendererInitContext.projectSettings->get_subsettings<ecore::RendererSubsettings>());
	_mainWindow = rendererInitContext.mainWindow;
	
	init_global_objects();
	init_module_objects();
	
	LOG_INFO("Renderer::init(): Initialized UIBackendCallbacks")
	
	_frameData.init();
	LOG_INFO("Renderer::init(): Initialized FrameData")

	LOG_INFO("Renderer::init(): GPU name: {}", RHI()->get_gpu_name())
	LOG_INFO("Renderer::init(): Driver description: {}", RHI()->get_driver_description())
}

void Renderer::cleanup()
{
	RHI()->wait_for_gpu();
	RENDER_GRAPH()->cleanup();
	IMGUI_BACKEND()->cleanup();
	RHI()->cleanup();
	ModuleObjects::cleanup();
}

void Renderer::bake()
{
	LOG_INFO("Renderer::bake(): Start baking")

	RenderingInitContext renderingInitContext;
	renderingInitContext.mainWindow = _mainWindow;
	
	_renderPassExecutors.emplace_back(new GBuffer(renderingInitContext));
	_renderPassExecutors.emplace_back(new DeferredLighting(renderingInitContext));
	_renderPassExecutors.emplace_back(new Culling(renderingInitContext));

	if (RENDERER_SUBSETTINGS()->get_scene_culling_settings().isOcclusionCullingEnabled)
		_renderPassExecutors.emplace_back(new DepthReduce(renderingInitContext));
	
	for (auto& executor : _renderPassExecutors)
		executor->prepare_render_pass();

	auto swapChainPass = std::make_unique<SwapChainPass>(renderingInitContext);
	swapChainPass->prepare_render_pass();
	RENDER_GRAPH()->set_swap_chain_executor(swapChainPass.get());
	_renderPassExecutors.push_back(std::move(swapChainPass));

	RENDER_GRAPH()->set_swap_chain_input("DeferredLightingOutput");
	RENDER_GRAPH()->bake();

	PIPELINE_MANAGER()->create_builtin_pipelines();

	RENDER_GRAPH()->log();
	
	set_backbuffer("DeferredLightingOutput");
	LOG_INFO("Renderer::bake(): Finished baking")
}

void Renderer::draw(DrawContext& drawContext)
{
	RHI()->reset_cmd_buffers(FRAME_INDEX);
	
	profiler::Profiler::begin_gpu_frame();

	SCENE_MANAGER()->setup_global_buffers();
	_frameData.update_uniform_buffers(drawContext);

	tasks::TaskGroup taskGroup;
	RENDER_GRAPH()->draw(&taskGroup);

	profiler::Profiler::end_gpu_frame();
	
	RHI()->submit(rhi::QueueType::GRAPHICS);
	RHI()->present();
	get_next_frame_index();
}

void Renderer::init_global_objects()
{
	ecs::set_type_info_table(ECS_TYPE_INFO_TABLE());
	profiler::Profiler::init(PROFILER_INSTANCE());
	LOG_INFO("Renderer::init(): Initialized global objects")
}

void Renderer::init_module_objects()
{
	ModuleObjects::init_rhi_module(_mainWindow);
	LOG_INFO("Renderer::init(): Loaded and initialized RHI module")
	
	ModuleObjects::init_render_core_module();
	LOG_INFO("Renderer::init(): Loaded and initialized RenderCore module")
	
	ModuleObjects::init_scene_manager();
	LOG_INFO("Renderer::init(): Initialized scene manager")

	ModuleObjects::init_imgui_backend(_mainWindow);
	LOG_INFO("Renderer::init(): Initialized ui backend")
}

void Renderer::get_next_frame_index()
{
	if (++FRAME_INDEX > RHI()->get_buffer_count() - 1)
		FRAME_INDEX = 0;
}

void Renderer::set_backbuffer(const std::string& textureName)
{
	IMGUI_BACKEND()->set_backbuffer(RENDERER_RESOURCE_MANAGER()->get_texture_view(textureName), SCENE_MANAGER()->get_sampler(SAMPLER_LINEAR_CLAMP));
}
