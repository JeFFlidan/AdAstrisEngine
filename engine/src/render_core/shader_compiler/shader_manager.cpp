#include "shader_manager.h"
#include "shader_compiler.h"
#include "file_system/utils.h"
#include "core/module_manager.h"
#include "profiler/logger.h"

using namespace ad_astris;
using namespace rcore::impl;

void ShaderManager::init(ShaderManagerInitContext& shaderManagerInitContext)
{
	_rhi = shaderManagerInitContext.rhi;
	
	ShaderCompilerInitContext shaderCompilerInitContext;
	shaderCompilerInitContext.cacheType = shaderManagerInitContext.cacheType;
	
	_shaderCache = ShaderCache(shaderCompilerInitContext);
	_shaderCompiler = std::make_unique<ShaderCompiler>(shaderCompilerInitContext);
}

rhi::Shader* ShaderManager::load_shader(
	const io::URI& relativeShaderPath,
	rhi::ShaderType shaderType,
	bool isEngineShader,
	rhi::HLSLShaderModel minHlslShaderModel,
	const std::vector<std::string>& shaderDefines)
{
	auto it = _shaderByRelativePath.find(relativeShaderPath.c_str());
	if (it != _shaderByRelativePath.end())
		return it->second.get();

	io::URI rootPath = isEngineShader ? FILE_SYSTEM()->get_engine_root_path() : FILE_SYSTEM()->get_project_root_path();

	if (_shaderCache.is_shader_outdated(relativeShaderPath, isEngineShader))
	{
		// LOG_INFO("SHADER {} IS OUTDATED", relativeShaderPath.c_str())
		rhi::ShaderFormat shaderFormat{ rhi::ShaderFormat::UNDEFINED };
		std::string shaderExtension = io::Utils::get_file_extension(relativeShaderPath);
		if (shaderExtension == "hlsl")
		{
			switch (_shaderCache.get_cache_type())
			{
				case ShaderCacheType::SPIRV:
					shaderFormat = rhi::ShaderFormat::HLSL_TO_SPIRV;
					break;
				case ShaderCacheType::DXIL:
					shaderFormat = rhi::ShaderFormat::HLSL6;
					break;
			}
		}
		else
		{
			switch (_shaderCache.get_cache_type())
			{
				case ShaderCacheType::SPIRV:
					shaderFormat = rhi::ShaderFormat::GLSL_TO_SPIRV;
					break;
				case ShaderCacheType::DXIL:
					shaderFormat = rhi::ShaderFormat::GLSL_TO_HLSL6;
					break;
			}
		}
		
		ShaderInputDesc inputDesc;
		inputDesc.shaderPath = rootPath + "/" + relativeShaderPath;
		inputDesc.format = shaderFormat;
		inputDesc.type = shaderType;
		inputDesc.minHlslShaderModel = minHlslShaderModel;
		inputDesc.defines = shaderDefines;
		inputDesc.includePaths.push_back((FILE_SYSTEM()->get_engine_root_path() + "/engine/shaders").c_str());
		if (!isEngineShader)
			inputDesc.includePaths.push_back(FILE_SYSTEM()->get_project_root_path().c_str());

		ShaderOutputDesc outputDesc;
		_shaderCompiler->compile(inputDesc, outputDesc);
		_shaderCache.update_shader_cache(inputDesc, outputDesc, isEngineShader);

		rhi::ShaderInfo shaderInfo;
		shaderInfo.shaderType = shaderType;
		shaderInfo.data = const_cast<uint8_t*>(outputDesc.data);
		shaderInfo.size = outputDesc.dataSize;

		_shaderByRelativePath[relativeShaderPath.c_str()] = std::make_unique<rhi::Shader>();
		_rhi->create_shader(_shaderByRelativePath[relativeShaderPath.c_str()].get(), &shaderInfo);
	}
	else
	{
		// LOG_INFO("LOAD SHADER")
		std::vector<uint8_t> shaderData;
		_shaderCache.load_shader_bin(io::Utils::get_file_name(relativeShaderPath), shaderData, isEngineShader);

		rhi::ShaderInfo shaderInfo;
		shaderInfo.data = shaderData.data();
		shaderInfo.size = shaderData.size();
		shaderInfo.shaderType = shaderType;

		_shaderByRelativePath[relativeShaderPath.c_str()] = std::make_unique<rhi::Shader>();
		_rhi->create_shader(_shaderByRelativePath[relativeShaderPath.c_str()].get(), &shaderInfo);
	}

	return _shaderByRelativePath[relativeShaderPath.c_str()].get();
}

rhi::Shader* ShaderManager::get_shader(const io::URI& relativeShaderPath)
{
	auto it = _shaderByRelativePath.find(relativeShaderPath.c_str());
	if (it != _shaderByRelativePath.end())
		return it->second.get();
	LOG_FATAL("ShaderManager::get_shader(): There is no shader {}", relativeShaderPath.c_str())
}
