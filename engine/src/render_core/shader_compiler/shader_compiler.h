#pragma once

#include "internal_compiler.h"
#include "shader_cache.h"
#include "render_core/api.h"

namespace ad_astris::rcore::impl
{
	class RENDER_CORE_API ShaderCompiler : public IShaderCompiler
	{
		public:
			ShaderCompiler(ShaderCompilerInitContext& initContext);
			virtual void compile(ShaderInputDesc& inputDesc, ShaderOutputDesc& outputDesc) override;

		private:
			std::unique_ptr<IInternalCompiler> _hlslCompiler{ nullptr };
#if defined(GLSL_SUPPORT)
			std::unique_ptr<IInternalCompiler> _glslCompiler{ nullptr };
#endif
			std::unique_ptr<ShaderCache> _shaderCache{ nullptr };

			std::string get_shader_format(rhi::ShaderFormat format);
	};
}