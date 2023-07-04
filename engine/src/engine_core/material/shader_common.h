#pragma once

#include "engine_core/object_name.h"
#include "rhi/resources.h"

namespace ad_astris::ecore::shader
{
	struct NonCompiledShaderData
	{
		void* data;
		uint64_t size;
	};

	struct CompilationContext
	{
		NonCompiledShaderData nonCompiledShaderData;
		rhi::ShaderInfo* compiledShaderInfo;
		ObjectName* shaderName;
		bool isCompiled;
	};
}