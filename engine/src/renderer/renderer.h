#pragma once

#include "api.h"
#include "frame_data.h"
#include "enums.h"
#include "material_manager.h"
#include "core/non_copyable_non_movable.h"
#include "scene_manager/scene_manager.h"
#include "engine/renderer_module.h"
#include "resource_manager/resource_manager.h"
#include "engine/render_core_module.h"
#include "rhi/engine_rhi.h"
#include "rhi/ui_window_backend.h"

namespace ad_astris::renderer::impl
{
	class RENDERER_API Renderer final : public IRenderer, public NonCopyableNonMovable
	{
		public:
			Renderer() = default;
			virtual ~Renderer() override { }

			virtual void init(RendererInitializationContext& rendererInitContext) override;
			virtual void cleanup() override;
		
			virtual rhi::IEngineRHI* get_rhi() override { return RHI(); }

			virtual void bake() override;
			virtual void draw(DrawContext& drawContext) override;
		
		private:
			//std::unique_ptr<MaterialManager> _materialManager{ nullptr };
			ecore::RendererSubsettings* _rendererSubsettings{ nullptr };
			acore::IWindow* _mainWindow{ nullptr };

			std::vector<std::unique_ptr<rcore::IRenderPassExecutor>> _renderPassExecutors;
			FrameData _frameData;
		
			uint32_t _frameIndex{ 0 };

			void init_global_objects(GlobalObjectContext* context);
			void init_module_objects();
			void get_current_frame_index();
			void set_backbuffer(const std::string& textureName);
	};
}
