#include "shader_cache.h"
#include "file_system/file.h"
#include "file_system/utils.h"
#include "profiler/logger.h"
#include <json/json.hpp>

using namespace ad_astris;
using namespace rcore;
using namespace impl;

ShaderCache::ShaderCache(ShaderCompilerInitContext& initContext) : _cacheType(initContext.cacheType)
{
	
}

// Took the idea from the WickedEngine
bool ShaderCache::is_shader_outdated(const io::URI& shaderRelativePath, bool isEngineShader)
{
	std::string shaderName = io::Utils::get_file_name(shaderRelativePath);
	io::URI shaderBinObjectRelativePath = "intermediate/shader_cache";

	switch (_cacheType)
	{
		case ShaderCacheType::DXIL:
			// TODO
			break;
		case ShaderCacheType::SPIRV:
			shaderBinObjectRelativePath += "/spirv/" + shaderName + ".spv";
			break;
	}

	io::URI shaderMetadataRelativePath = "intermediate/shader_cache/metadata/" + shaderName + ".aameta";
	io::URI rootPath = isEngineShader ? FILE_SYSTEM()->get_engine_root_path() : FILE_SYSTEM()->get_project_root_path();
	
	io::URI shaderSourceAbsolutePath = io::Utils::get_absolute_path_to_file(rootPath, shaderRelativePath);
	io::URI shaderBinObjectAbsolutePath = io::Utils::get_absolute_path_to_file(rootPath, shaderBinObjectRelativePath);
	io::URI shaderBinObjectMetadataPath = io::Utils::get_absolute_path_to_file(rootPath, shaderMetadataRelativePath);

	if (!io::Utils::exists(shaderBinObjectAbsolutePath) || !io::Utils::exists(shaderBinObjectMetadataPath))
		return true;

	uint64_t shaderSourceTimeStamp = io::Utils::get_last_write_time(shaderSourceAbsolutePath);
	uint64_t shaderBinObjectTimeStamp = io::Utils::get_last_write_time(shaderBinObjectAbsolutePath);

	if (shaderBinObjectTimeStamp < shaderSourceTimeStamp)
		return true;
	
	std::vector<uint8_t> outputData;
	io::Utils::read_file(FILE_SYSTEM(), shaderBinObjectMetadataPath, outputData);
	std::string strMetadata;
	strMetadata.resize(outputData.size());
	memcpy(strMetadata.data(), outputData.data(), outputData.size());

	nlohmann::json shaderBinObjectMetadata = nlohmann::json::parse(strMetadata);
	uint64_t dependencyTimeStamp = 0;
	for (auto& keyAndValue : shaderBinObjectMetadata.items())
	{
		std::string dependencyAbsolutePath = keyAndValue.key();
		if (io::Utils::exists(dependencyAbsolutePath))
		{
			dependencyTimeStamp = io::Utils::get_last_write_time(dependencyAbsolutePath);

			if (shaderBinObjectTimeStamp < dependencyTimeStamp)
				return true;
		}
	}

	return false;
}

void ShaderCache::update_shader_cache(ShaderInputDesc& inputDesc, ShaderOutputDesc& outputDesc, bool isEngineShader)
{
	std::string shaderName = io::Utils::get_file_name(inputDesc.shaderPath.c_str());
	io::URI rootPath = isEngineShader ? FILE_SYSTEM()->get_engine_root_path() : FILE_SYSTEM()->get_project_root_path();

	io::URI shaderBinObjectRelativePath;
	get_shader_object_relative_path(shaderName, shaderBinObjectRelativePath);
	io::URI shaderMetadataRelativePath = "intermediate/shader_cache/metadata/" + shaderName + ".aameta";

	io::URI shaderBinObjectAbsolutePath = io::Utils::get_absolute_path_to_file(rootPath, shaderBinObjectRelativePath);
	io::URI shaderBinObjectMetadataPath = io::Utils::get_absolute_path_to_file(rootPath, shaderMetadataRelativePath);
	
	io::Utils::write_file(FILE_SYSTEM(), shaderBinObjectAbsolutePath, outputDesc.data, outputDesc.dataSize);

	nlohmann::json shaderMetadata;
	for (auto& dependency : outputDesc.dependencies)
	{
		shaderMetadata[dependency] = 0;
	}

	std::string strShaderMetadata = shaderMetadata.dump();
	io::Utils::write_file(FILE_SYSTEM(), shaderBinObjectMetadataPath, strShaderMetadata.c_str(), strShaderMetadata.size());
}

void ShaderCache::load_shader_bin(const std::string& shaderName, std::vector<uint8_t>& outputData, bool isEngineShader)
{
	io::URI relativePath;
	get_shader_object_relative_path(const_cast<std::string&>(shaderName), relativePath);
	io::URI absolutePath = isEngineShader ? FILE_SYSTEM()->get_engine_root_path() : FILE_SYSTEM()->get_project_root_path();
	absolutePath = io::Utils::get_absolute_path_to_file(absolutePath, relativePath);
	io::Utils::read_file(FILE_SYSTEM(), absolutePath, outputData);
}

void ShaderCache::get_shader_object_relative_path(std::string& shaderName, io::URI& output)
{
	output= "intermediate/shader_cache";

	switch (_cacheType)
	{
		case ShaderCacheType::DXIL:
			// TODO
			break;
		case ShaderCacheType::SPIRV:
			output += "/spirv/" + shaderName + ".spv";
		break;
	}
}
