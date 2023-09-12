#pragma once

#include "shader_cache.h"
#include "rhi/resources.h"
#include "engine/render_core_module.h"
#include "file_system/file_system.h"
#include "engine_core/material/shader_common.h"
#include <shaderc/shaderc/shaderc.h>

namespace ad_astris::rcore::impl
{
	const uint32_t magicNumber = 0x07230203;
	
	// shaderc_include_result* RENDER_CORE_API include_resolver(
	// 	void* userData,
	// 	const char* requested_source,
	// 	int type,
	// 	const char* requesting_source,
	// 	size_t includeDepth);
	//
	// void RENDER_CORE_API include_releaser(void* userData, shaderc_include_result* result);
	
	class RENDER_CORE_API ShaderCompiler : public IShaderCompiler
	{
		public:
			ShaderCompiler() = default;

			virtual void init(io::FileSystem* fileSystem) override;
			// Legacy
			virtual void compile_into_spv(io::URI& path, rhi::ShaderInfo* info) override;
		
			virtual void compile_shader_into_spv(ecore::shader::CompilationContext& compilationContext) override;
		
		private:
			io::FileSystem* _fileSystem;
			ShaderCache _shaderCache;
			shaderc_compiler_t _compiler { nullptr };
			shaderc_compile_options_t _options { nullptr };
		
			shaderc_shader_kind get_shader_kind(rhi::ShaderType shaderType);
			rhi::ShaderType get_shader_type(io::URI& path);
	};
}