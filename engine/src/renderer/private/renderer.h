#pragma once

#include "api.h"
#include "frame_data.h"
#include "enums.h"
#include "core/non_copyable_non_movable.h"
#include "scene_manager/scene_manager.h"
#include "renderer/public/renderer_module.h"
#include "resource_manager/resource_manager.h"
#include "render_core/public/render_core_module.h"
#include "rhi/engine_rhi.h"

namespace ad_astris::renderer::impl
{
	class RENDERER_API Renderer final : public IRenderer, public NonCopyableNonMovable
	{
		public:
			Renderer() = default;
			virtual ~Renderer() override { }

			virtual void init(RendererInitializationContext& rendererInitContext) override;
			virtual void cleanup() override;
		
			virtual rhi::RHI* get_rhi() override { return RHI(); }

			virtual void bake() override;
			virtual void draw(DrawContext& drawContext) override;
		
		private:
			//std::unique_ptr<MaterialManager> _materialManager{ nullptr };
			ecore::RendererSubsettings* _rendererSubsettings{ nullptr };
			acore::IWindow* _mainWindow{ nullptr };

			std::vector<std::unique_ptr<rcore::IRenderPassExecutor>> _renderPassExecutors;
			FrameData _frameData;

			void init_global_objects();
			void init_module_objects();
			void get_next_frame_index();
			void set_backbuffer(const std::string& textureName);
	};
}
