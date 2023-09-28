#pragma once

#include "rhi/engine_rhi.h"
#include "scene_manager/scene_manager.h"
#include "engine/render_core_module.h"

// GPU means that data is for shaders, CPU means that data shouldn't be sent to the GPU
namespace ad_astris::renderer::impl
{
	struct RenderingInitContext
	{
		rhi::IEngineRHI* rhi;
		SceneManager* sceneManager;
		acore::IWindow* mainWindow;
		rcore::IPipelineManager* pipelineManager;
	};

	class RenderingBase
	{
		public:
			RenderingBase(RenderingInitContext& initContext) : _rhi(initContext.rhi),
				_sceneManager(initContext.sceneManager), _mainWindow(initContext.mainWindow), _pipelineManager(initContext.pipelineManager) { }

		protected:
			rhi::IEngineRHI* _rhi{ nullptr};
			SceneManager* _sceneManager{ nullptr };
			acore::IWindow* _mainWindow{ nullptr };
			rcore::IPipelineManager* _pipelineManager{ nullptr };
	};
}