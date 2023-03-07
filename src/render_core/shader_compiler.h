#pragma once

#include "rhi/resources.h"
#include "file_system/IO.h"
#include "shader_cache.h"
#include <shaderc/shaderc.h>

namespace ad_astris::rcore
{
	const uint32_t magicNumber = 0x07230203;
	
	shaderc_include_result* include_resolver(
		void* userData,
		const char* requested_source,
		int type,
		const char* requesting_source,
		size_t includeDepth);

	void include_releaser(void* userData, shaderc_include_result* result);
	
	class ShaderCompiler
	{
		public:
			ShaderCache _shaderCache;
		
			ShaderCompiler() = default;
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