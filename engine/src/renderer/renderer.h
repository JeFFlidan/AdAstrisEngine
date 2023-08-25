#pragma once

#include "api.h"
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
		
		private:
			ecore::RendererSubsettings* _rendererSubsettings{ nullptr };
			rhi::IEngineRHI* _rhi{ nullptr };
			rcore::IRenderGraph* _renderGraph{ nullptr };
			rcore::IShaderCompiler* _shaderCompiler{ nullptr };
			resource::ResourceManager* _resourceManager{ nullptr };
			events::EventManager* _eventManager{ nullptr };
			tasks::TaskComposer* _taskComposer{ nullptr };
	};
}
