#include "shader_compiler.h"
#include "profiler/logger.h"
#include <string>

using namespace ad_astris;

rcore::ShaderCompiler::ShaderCompiler(io::FileSystem* fileSystem) : _fileSystem(fileSystem)
{
	_compiler = shaderc_compiler_initialize();
	_options = shaderc_compile_options_initialize();
}

void rcore::ShaderCompiler::compile_into_spv(io::URI& uri, rhi::ShaderInfo* info)
{
	rhi::ShaderType shaderType = get_shader_type(uri);
	info->shaderType = shaderType;
	uint64_t count;
	void* data = _fileSystem->map_to_system(uri, count);
	shaderc_shader_kind shaderKind = get_shader_kind(shaderType);
	shaderc_compilation_result_t preprocessResult = nullptr;
	preprocessResult = shaderc_compile_into_preprocessed_text(
		_compiler,
		static_cast<char*>(data),
		count,
		shaderKind,
		uri.c_str(),
		"main",
		_options);

	if (shaderc_result_get_compilation_status(preprocessResult) != shaderc_compilation_status_success)
	{
		LOG_ERROR("ShaderCompiler::compile_to_spirv(): failed to precompile shader {}", uri.c_str())
		LOG_ERROR("Compilation error: {}", shaderc_result_get_error_message(preprocessResult))
		shaderc_result_release(preprocessResult);
		return;
	}

	size_t codeLength = shaderc_result_get_length(preprocessResult);
	LOG_INFO("Code length: {}", codeLength)
	const char* codeBin = shaderc_result_get_bytes(preprocessResult);

	// I have to implement shader cache in the future

	shaderc_compilation_result_t finalResult = nullptr;
	finalResult = shaderc_compile_into_spv(
		_compiler,
		codeBin,
		codeLength,
		shaderKind,
		uri.c_str(),
		"main",
		_options);
	
	shaderc_result_release(preprocessResult);
	_fileSystem->unmap_from_system(data);
	
	if (shaderc_result_get_compilation_status(finalResult) != shaderc_compilation_status_success)
	{
		LOG_ERROR("ShaderCompiler::compile_to_spirv(): failed to compile shader {}", uri.c_str())
		LOG_ERROR("Compilation error: {}", shaderc_result_get_error_message(finalResult))
		shaderc_result_release(finalResult);
		return;
	}

	info->size = shaderc_result_get_length(finalResult);
	const uint8_t* code = reinterpret_cast<const uint8_t*>(shaderc_result_get_bytes(finalResult));
	info->data = const_cast<uint8_t*>(code);
	shaderc_result_release(finalResult);
}

// private methods
shaderc_shader_kind rcore::ShaderCompiler::get_shader_kind(rhi::ShaderType shaderType)
{
	switch (shaderType)
	{
		case rhi::VERTEX:
			return shaderc_vertex_shader;
		case rhi::FRAGMENT:
			return shaderc_fragment_shader;
		case rhi::TESSELLATION_CONTROL:
			return shaderc_tess_control_shader;
		case rhi::TESSELLATION_EVALUATION:
			return shaderc_tess_evaluation_shader;
		case rhi::GEOMETRY:
			return shaderc_geometry_shader;
		case rhi::COMPUTE:
			return shaderc_compute_shader;
		case rhi::MESH:
			return shaderc_mesh_shader;
		case rhi::TASK:
			return shaderc_task_shader;
		case rhi::RAY_GENERATION:
			return shaderc_raygen_shader;
		case rhi::RAY_INTERSECTION:
			return shaderc_intersection_shader;
		case rhi::RAY_ANY_HIT:
			return shaderc_anyhit_shader;
		case rhi::RAY_CLOSEST_HIT:
			return shaderc_closesthit_shader;
		case rhi::RAY_MISS:
			return shaderc_miss_shader;
		case rhi::RAY_CALLABLE:
			return shaderc_callable_shader;
	}
}

rhi::ShaderType rcore::ShaderCompiler::get_shader_type(io::URI& path)
{
	std::string extension(std::filesystem::path(path.c_str()).extension().string().erase(0, 1));
	LOG_INFO("Shader extensions: {}", extension.c_str())

	if (extension == "vert")
		return rhi::VERTEX;
	if (extension == "frag")
		return rhi::FRAGMENT;
	if (extension == "tesc")
		return rhi::TESSELLATION_CONTROL;
	if (extension == "tese")
		return rhi::TESSELLATION_EVALUATION;
	if (extension == "geom")
		return rhi::GEOMETRY;
	if (extension == "comp")
		return rhi::COMPUTE;
	if (extension == "mesh")
		return rhi::MESH;
	if (extension == "task")
		return rhi::TASK;
	if (extension == "rgen")
		return rhi::RAY_GENERATION;
	if (extension == "rint")
		return rhi::RAY_INTERSECTION;
	if (extension == "rahit")
		return rhi::RAY_ANY_HIT;
	if (extension == "rchit")
		return rhi::RAY_CLOSEST_HIT;
	if (extension == "rmiss")
		return rhi::RAY_MISS;
	if (extension == "rcall")
		return rhi::RAY_CALLABLE;

	LOG_ERROR("Unsopported shader extension {}", extension.c_str())
	return rhi::UNDEFINED_SHADER_TYPE;
}
