#include "shader.h"
#include "profiler/logger.h"
#include "file_system/utils.h"
#include "resource_manager/resource_visitor.h"

using namespace ad_astris;
using namespace ecore;

Shader::Shader(ObjectName* shaderName)
{
	_uuid = UUID();
	_name = shaderName;
}

void Shader::serialize(io::IFile* file)
{
    
}

Shader::~Shader()
{
	delete[] _shaderInfo.data;
}

void Shader::deserialize(io::IFile* file, ObjectName* objectName)
{
	_file = file;
	_shaderInfo.shaderType = get_shader_type_by_file_ext(file->get_file_path());
	_uuid = std::stoull(file->get_metadata());
	_name = objectName;
}

shader::CompilationContext Shader::get_compilation_context()
{
	shader::CompilationContext compilationContext;
	compilationContext.nonCompiledShaderData.data = _file->get_binary_blob();
	compilationContext.nonCompiledShaderData.size = _file->get_binary_blob_size();
	compilationContext.compiledShaderInfo = &_shaderInfo;
	compilationContext.shaderName = _name;
	compilationContext.isCompiled = _isCompiled;
	return compilationContext;
}

inline void Shader::accept(resource::IResourceVisitor& resourceVisitor)
{
	resourceVisitor.visit(this);
}

rhi::ShaderType Shader::get_shader_type_by_file_ext(const io::URI& path)
{
	std::string extension = io::Utils::get_file_extension(path);

	if (extension == "vert")
		return rhi::ShaderType::VERTEX;
	if (extension == "frag")
		return rhi::ShaderType::FRAGMENT;
	if (extension == "tesc")
		return rhi::ShaderType::TESSELLATION_CONTROL;
	if (extension == "tese")
		return rhi::ShaderType::TESSELLATION_EVALUATION;
	if (extension == "geom")
		return rhi::ShaderType::GEOMETRY;
	if (extension == "comp")
		return rhi::ShaderType::COMPUTE;
	if (extension == "mesh")
		return rhi::ShaderType::MESH;
	if (extension == "task")
		return rhi::ShaderType::TASK;
	if (extension == "rgen")
		return rhi::ShaderType::RAY_GENERATION;
	if (extension == "rint")
		return rhi::ShaderType::RAY_INTERSECTION;
	if (extension == "rahit")
		return rhi::ShaderType::RAY_ANY_HIT;
	if (extension == "rchit")
		return rhi::ShaderType::RAY_CLOSEST_HIT;
	if (extension == "rmiss")
		return rhi::ShaderType::RAY_MISS;
	if (extension == "rcall")
		return rhi::ShaderType::RAY_CALLABLE;

	LOG_ERROR("Unsopported shader extension {}", extension.c_str())
	return rhi::ShaderType::UNDEFINED;
}
