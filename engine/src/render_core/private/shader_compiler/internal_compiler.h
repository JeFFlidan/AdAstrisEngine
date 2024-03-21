#pragma once

#include "render_core/public/render_core_module.h"

namespace ad_astris::rcore::impl
{
	class IInternalCompiler
	{
		public:
			virtual void init(ShaderCompilerInitContext& initContext) = 0;
			virtual void compile(ShaderInputDesc& inputDesc, ShaderOutputDesc& outputDesc) = 0;
	};
}