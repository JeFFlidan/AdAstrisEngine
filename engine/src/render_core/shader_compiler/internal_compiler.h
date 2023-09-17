#pragma once

#include "engine/render_core_module.h"
#include "file_system/file_system.h"

namespace ad_astris::rcore::impl
{
	class IInternalCompiler
	{
		public:
			virtual void init(ShaderCompilerInitContext& initContext) = 0;
			virtual void compile(io::FileSystem* fileSystem, ShaderInputDesc& inputDesc, ShaderOutputDesc& outputDesc) = 0;
	};
}