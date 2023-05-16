#pragma once

#include "rhi/engine_rhi.h"
#include <memory>

namespace ad_astris::systems
{
	class Renderer
	{
		public:
			Renderer();
			Renderer(const Renderer& renderer) = delete;
			Renderer(const Renderer&& renderer) = delete;
			Renderer& operator=(const Renderer& renderer) = delete;
		
		private:
			rhi::IEngineRHI* _rhi;
	};
}
