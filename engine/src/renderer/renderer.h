#pragma once

#include "api.h"
#include "material_manager.h"
#include "scene_manager/scene_manager.h"
#include "engine/renderer_module.h"
#include "resource_manager/resource_manager.h"
#include "engine/render_core_module.h"
#include "rhi/engine_rhi.h"

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

			virtual void init(RendererInitializationContext& initializationContext) override;
			virtual void cleanup() override;

			virtual void bake() override;
			virtual void draw() override;
		
		private:
			std::unique_ptr<MaterialManager> _materialManager{ nullptr };
			std::unique_ptr<SceneManager> _sceneManager{ nullptr };
			ecore::RendererSubsettings* _rendererSubsettings{ nullptr };
			rhi::IEngineRHI* _rhi{ nullptr };
			rcore::IRenderGraph* _renderGraph{ nullptr };
			rcore::IShaderCompiler* _shaderCompiler{ nullptr };
			resource::ResourceManager* _resourceManager{ nullptr };
			events::EventManager* _eventManager{ nullptr };
			tasks::TaskComposer* _taskComposer{ nullptr };
			ecore::EngineObjectsCreator* _engineObjectsCreator{ nullptr };

			uint32_t _frameNumber{ 0 };

			uint32_t get_current_frame_index();

			void test_light_submanager();
			bool _wasLightSubmanagerTested{ false };
	};
}
