#include "engine/renderer_module.h"
#include "renderer.h"
#include <memory>

namespace ad_astris::renderer
{
	class RendererModule : public IRendererModule
	{
		public:
			virtual void startup_module(ModuleManager* moduleManager) override;
			virtual IRenderer* get_renderer() override;

		private:
			std::unique_ptr<IRenderer> _renderer;
	};

	void RendererModule::startup_module(ModuleManager* moduleManager)
	{
		_renderer = std::make_unique<impl::Renderer>();
	}

	IRenderer* RendererModule::get_renderer()
	{
		return _renderer.get();
	}

	DECL_MODULE_FUNCS(Renderer, RendererModule)
}