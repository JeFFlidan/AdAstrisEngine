#pragma once

#include "rhi/resources.h"
#include "file_system/IO.h"
#include <shaderc/shaderc.h>

namespace ad_astris::rcore
{
	class ShaderCompiler
	{
		public:
			ShaderCompiler(io::FileSystem* fileSystem);

			void compile_into_spv(io::URI& path, rhi::ShaderInfo* info);

		private:
			io::FileSystem* _fileSystem;
			shaderc_compiler_t _compiler { nullptr };
			shaderc_compile_options_t _options { nullptr };
		
			shaderc_shader_kind get_shader_kind(rhi::ShaderType shaderType);
			rhi::ShaderType get_shader_type(io::URI& path);
	};
}