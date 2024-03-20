#include "shader_compiler.h"
#include "dx_internal_compiler.h"

#if defined(GLSL_SUPPORT)
#include "glsl_internal_compiler.h"
#endif

#include "profiler/logger.h"

using namespace ad_astris;
using namespace rcore;
using namespace impl;

ShaderCompiler::ShaderCompiler(ShaderCompilerInitContext& initContext)
{
#if defined(DXCOMPILER_ENABLED)
	_hlslCompiler = std::make_unique<DXInternalCompiler>();
	_hlslCompiler->init(initContext);
#endif
	
#if defined(GLSL_SUPPORT)
	_glslCompiler = std::make_unique<GLSLInternalCompiler>();
	_glslCompiler->init(initContext);
#endif

	_shaderCache = std::make_unique<ShaderCache>(initContext);
}

void ShaderCompiler::compile(ShaderInputDesc& inputDesc, ShaderOutputDesc& outputDesc)
{
#if defined(DXCOMPILER_ENABLED)
	if (inputDesc.format == rhi::ShaderFormat::HLSL6 || inputDesc.format == rhi::ShaderFormat::HLSL_TO_SPIRV)
	{
		_hlslCompiler->compile(inputDesc, outputDesc);
		return;
	}
#endif

#if defined(GLSL_SUPPORT)
	if (inputDesc.format == rhi::ShaderFormat::GLSL_TO_HLSL6 || inputDesc.format == rhi::ShaderFormat::GLSL_TO_SPIRV)
	{
		_glslCompiler->compile(inputDesc, outputDesc);
		return;
	}
#endif

	LOG_ERROR("ShaderCompiler::compile(): Shader hasn't been compiled because there is no compiler that can build {}", get_shader_format(inputDesc.format))
}

std::string ShaderCompiler::get_shader_format(rhi::ShaderFormat format)
{
	switch (format)
	{
		case rhi::ShaderFormat::HLSL6:
			return "HLSL6";
		case rhi::ShaderFormat::GLSL_TO_HLSL6:
			return "GLSL_TO_HLSL";
		case rhi::ShaderFormat::GLSL_TO_SPIRV:
			return "GLSL_TO_SPIRV";
		case rhi::ShaderFormat::HLSL_TO_SPIRV:
			return "HLSL_TO_SPIRV";
	}
}
