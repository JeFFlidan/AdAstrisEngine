#pragma once

#include "renderer.h"
#include "ui_backend.h"

namespace ad_astris::renderer
{
	class IRendererModule : public IModule
	{
		public:
			virtual IRenderer* get_renderer() const = 0;
			virtual IUiBackend* get_ui_backend() const = 0;
	};
}