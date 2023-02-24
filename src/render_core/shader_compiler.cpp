#include "shader_compiler.h"
#include "profiler/logger.h"
#include <string>

using namespace ad_astris;

shaderc_include_result* rcore::include_resolver(
	void* userData,
	const char* requested_source,
	int type,
	const char* requesting_source,
	size_t includeDepth)
{
	// LOG_INFO("Requested source {}", requested_source)
	// LOG_INFO("Requesting source {}", requesting_source)
	// LOG_INFO("Include depth {}", includeDepth)

	io::FileSystem* fileSystem = static_cast<io::FileSystem*>(userData);
	assert(fileSystem && "Invalid file system");

	shaderc_include_result* includeResult = new shaderc_include_result();
	includeResult->user_data = userData;
	includeResult->source_name = nullptr;
	includeResult->content = nullptr;
	includeResult->source_name_length = 0;
	includeResult->content_length = 0;

	std::string includePath;
	
	switch (type)
	{
		case shaderc_include_type_relative:
		{
			LOG_INFO("Relative")
			std::string shaderPath(requesting_source);
			size_t pos = shaderPath.find_last_of("/\\");
			includePath = shaderPath.substr(0, pos + 1);
			break;
		}
	}

	includePath += std::string(requested_source);

	uint64_t size;
	void* data = fileSystem->map_to_system(includePath.c_str(), size);

	if (!data)
	{
		LOG_ERROR("ShaderCompiler::include_resolver(): invalid data after reading from file {}", includePath)
		return includeResult;
	}

	char* path = new char[includePath.size() + 1];
	memcpy(path, includePath.c_str(), includePath.size());
	path[includePath.size()] = '\x0';

	includeResult->source_name = path;
	includeResult->source_name_length = includePath.size();
	includeResult->content = static_cast<const char*>(data);
	includeResult->content_length = size;

	return includeResult;
}

void rcore::include_releaser(void* userData, shaderc_include_result* result)
{
	io::FileSystem* fileSystem = static_cast<io::FileSystem*>(userData);
	assert(fileSystem && "Invalid file system");

	fileSystem->unmap_from_system(const_cast<char*>(result->content));

	delete[] result->source_name;
	delete result;
}

rcore::ShaderCompiler::ShaderCompiler(io::FileSystem* fileSystem) : _fileSystem(fileSystem)
{
	_compiler = shaderc_compiler_initialize();
	_options = shaderc_compile_options_initialize();

	shaderc_compile_options_set_target_env(_options, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
	shaderc_compile_options_set_target_spirv(_options, shaderc_spirv_version_1_4);
	shaderc_compile_options_set_source_language(_options, shaderc_source_language_glsl);
	shaderc_compile_options_set_include_callbacks(_options, include_resolver, include_releaser, _fileSystem);
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
	const uint32_t* code = reinterpret_cast<const uint32_t*>(shaderc_result_get_bytes(finalResult));
	info->data = const_cast<uint32_t*>(code);
	
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
