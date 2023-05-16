#include "shader_cache.h"
#include "profiler/logger.h"
#include <functional>

using namespace ad_astris;

rcore::ShaderCache::~ShaderCache()
{
	LOG_INFO("Shader cache destructor")
	for (auto& it : _cache)
	{
		LOG_INFO("Delete all")
		if (it.second.data)
		{
			delete[] it.second.data;
			it.second.data = nullptr;
		}
	}
}

bool rcore::ShaderCache::check_in_cache(rhi::ShaderInfo* shaderInfo, void* data, uint64_t size)
{
	DataFromFile dataFromFile(static_cast<uint8_t*>(data), size);
	auto it = _cache.find(dataFromFile);
	if (it != _cache.end())
	{
		rhi::ShaderInfo cachedInfo = it->second;
		shaderInfo->data = cachedInfo.data;
		shaderInfo->size = cachedInfo.size;
		shaderInfo->shaderType = cachedInfo.shaderType;
		return true;
	}
	return false;
}

void rcore::ShaderCache::add_to_cache(rhi::ShaderInfo* shaderInfo, void* dataFromFile, uint64_t dataSize)
{
	DataFromFile key(static_cast<uint8_t*>(dataFromFile), dataSize);
	rhi::ShaderInfo newInfo;
	newInfo.shaderType = shaderInfo->shaderType;
	newInfo.size = shaderInfo->size;
	newInfo.data = shaderInfo->data;
	_cache[key] = newInfo;
}

//private methods

rcore::ShaderCache::DataFromFile::DataFromFile(uint8_t* data, uint64_t size) : size(size)
{
	uint8_t devider = 2;
	for (int i = 0; i != 8; ++i)
	{
		bytes[i] = data[size / devider] << devider;
		++devider;
	}
}

bool rcore::ShaderCache::DataFromFile::operator==(const DataFromFile& other) const
{
	if (other.size != size)
	{
		return false;
	}

	for (size_t i = 0; i != 8; ++i)
	{
		if (bytes[i] != other.bytes[i])
		{
			return false;
		}
	}
	
	return true;
}

size_t rcore::ShaderCache::DataFromFile::hash() const
{
	size_t result = std::hash<size_t>()(size);
	size_t codeHash = bytes[0] | bytes[1] | bytes[2] | bytes[3] | bytes[4] | bytes[5] | bytes[6] | bytes[7];
	result ^= std::hash<size_t>()(codeHash);
	return result;
}

