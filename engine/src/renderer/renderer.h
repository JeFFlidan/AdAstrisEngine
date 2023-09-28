#pragma once

#include "api.h"
#include "enums.h"
#include "material_manager.h"
#include "scene_manager/scene_manager.h"
#include "engine/renderer_module.h"
#include "resource_manager/resource_manager.h"
#include "engine/render_core_module.h"
#include "rhi/engine_rhi.h"
#include "rhi/ui_window_backend.h"

namespace ad_astris::renderer::impl
{
	class RENDERER_API Renderer final : public IRenderer
	{
		public:
			Renderer() = default;
			virtual ~Renderer() override { }
			Renderer(const Renderer& renderer) = delete;
			Renderer(const Renderer&& renderer) = delete;
			Renderer& operator=(const Renderer& renderer) = delete;
			Renderer& operator=(const Renderer&& renderer) = delete;

			virtual void init(RendererInitializationContext& rendererInitContext) override;
			virtual void cleanup() override;

			virtual void set_new_world(ecore::World* world) override { _world = world; }

			virtual void bake() override;
			virtual void draw(DrawContext& drawContext) override;
		
		private:
			//std::unique_ptr<MaterialManager> _materialManager{ nullptr };
			std::unique_ptr<SceneManager> _sceneManager{ nullptr };
			ecore::RendererSubsettings* _rendererSubsettings{ nullptr };
			rhi::IEngineRHI* _rhi{ nullptr };
			rhi::UIWindowBackend* _uiWindowBackend{ nullptr };
			rcore::IRenderGraph* _renderGraph{ nullptr };
			rcore::IShaderManager* _shaderManager{ nullptr };
			resource::ResourceManager* _resourceManager{ nullptr };
			rcore::IRendererResourceManager* _rendererResourceManager{ nullptr };
			rcore::IPipelineManager* _pipelineManager{ nullptr };
			events::EventManager* _eventManager{ nullptr };
			tasks::TaskComposer* _taskComposer{ nullptr };
			acore::IWindow* _mainWindow{ nullptr };
			ecore::World* _world{ nullptr };

			std::vector<std::unique_ptr<rcore::IRenderPassExecutor>> _renderPassExecutors;
		
			uint32_t _frameIndex{ 0 };

			void get_current_frame_index();
			void create_uniform_buffers();
			void setup_cameras(DrawContext& preDrawContext);
			void setup_frame_data(DrawContext& preDrawContext);
			void set_backbuffer(const std::string& textureName);
	};
}
