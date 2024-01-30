#pragma once

#include "rhi/engine_rhi.h"
#include "rhi/ui_window_backend.h"
#include "engine/render_core_module.h"
#include "engine_core/project_settings/renderer_subsettings.h"
#include "core/common.h"

namespace ad_astris::renderer::impl
{
	class SceneManager;
	
	class ModuleObjects
	{
		public:
			static void init_rhi_module(ecore::RendererSubsettings* rendererSubsettings, acore::IWindow* mainWindow);
			static void init_render_core_module(ecore::RendererSubsettings* rendererSubsettings);
			static void init_scene_manager();
			static void init_ui_window_backend(acore::IWindow* mainWindow);
			static void cleanup();

			FORCE_INLINE static rhi::IEngineRHI* get_rhi() { return _rhi; }
			FORCE_INLINE static rhi::UIWindowBackend* get_ui_window_backend() { return _uiWindowBackend; }
			FORCE_INLINE static rcore::IRenderGraph* get_render_graph() { return _renderGraph; }
			FORCE_INLINE static rcore::IShaderManager* get_shader_manager() { return _shaderManager; }
			FORCE_INLINE static rcore::IPipelineManager* get_pipeline_manager() { return _pipelineManager; }
			FORCE_INLINE static rcore::IRendererResourceManager* get_renderer_resource_manager() { return _rendererResourceManager; }
			FORCE_INLINE static SceneManager* get_scene_manager() { return _sceneManager; }
		
		private:
			inline static rhi::IEngineRHI* _rhi{ nullptr };
			inline static rhi::UIWindowBackend* _uiWindowBackend{ nullptr };
			inline static rcore::IRenderGraph* _renderGraph{ nullptr };
			inline static rcore::IShaderManager* _shaderManager{ nullptr };
			inline static rcore::IPipelineManager* _pipelineManager{ nullptr };
			inline static rcore::IRendererResourceManager* _rendererResourceManager{ nullptr };
			inline static SceneManager* _sceneManager{ nullptr };	// Use no unique_ptr and fwd declaration of SceneManager to avoid circular dependency 
	};
}

#define RHI() ::ad_astris::renderer::impl::ModuleObjects::get_rhi()
#define PIPELINE_MANAGER() ::ad_astris::renderer::impl::ModuleObjects::get_pipeline_manager()
#define RENDERER_RESOURCE_MANAGER() ::ad_astris::renderer::impl::ModuleObjects::get_renderer_resource_manager()
#define UI_WINDOW_BACKEND() ::ad_astris::renderer::impl::ModuleObjects::get_ui_window_backend()
#define SHADER_MANAGER() ::ad_astris::renderer::impl::ModuleObjects::get_shader_manager()
#define RENDER_GRAPH() ::ad_astris::renderer::impl::ModuleObjects::get_render_graph()
#define SCENE_MANAGER() ::ad_astris::renderer::impl::ModuleObjects::get_scene_manager()