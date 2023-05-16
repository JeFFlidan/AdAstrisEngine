#pragma once

#include "rhi/resources.h"
#include "rhi/engine_rhi.h"

namespace ad_astris::renderer
{
	class SceneManager
	{
		public:
			SceneManager(rhi::IEngineRHI* rhi);
			~SceneManager();
		
		private:
			rhi::IEngineRHI* _rhi;
	};
}