#include "renderer.h"
#include "ui_backend/ui_backend.h"
#include "renderer/public/renderer_module.h"
#include <memory>

namespace ad_astris::renderer
{
	class RendererModule : public IRendererModule
	{
		public:
			virtual void startup_module(ModuleManager* moduleManager) override
			{
				_renderer = std::make_unique<impl::Renderer>();
				_uiBackend = std::make_unique<impl::UiBackend>();
			}
		
			virtual IRenderer* get_renderer() const override
			{
				return _renderer.get();
			}
		
			virtual IUiBackend* get_ui_backend() const override
			{
				return _uiBackend.get();
			}

		private:
			std::unique_ptr<IRenderer> _renderer{ nullptr };
			std::unique_ptr<IUiBackend> _uiBackend{ nullptr };
	};

	DECL_MODULE_FUNCS(Renderer, RendererModule)
}
