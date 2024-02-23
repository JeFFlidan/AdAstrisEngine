#pragma once

#include "scene_manager/scene_manager.h"

// GPU means that data is for shaders, CPU means that data shouldn't be sent to the GPU
namespace ad_astris::renderer::impl
{
	struct RenderingInitContext
	{
		acore::IWindow* mainWindow;
	};

	class RenderingBase
	{
		public:
			RenderingBase(RenderingInitContext& initContext) : _mainWindow(initContext.mainWindow) { }

		protected:
			acore::IWindow* _mainWindow{ nullptr };
	};
}