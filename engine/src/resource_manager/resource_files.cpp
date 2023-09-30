#include "resource_files.h"
#include "resource_visitor.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"
#include "engine_core/level/level.h"
#include <lz4/lz4.h>
#include <json/json.hpp>

using namespace ad_astris;
using namespace resource;

template<typename T>
ResourceFile::ResourceFile(ConversionContext<T>& context)
{
	
}

template<>
ResourceFile::ResourceFile(ConversionContext<ecore::StaticModel>& context)
{
	context.get_data(_metadata, _binBlob, _binBlobSize, _path);
}

template<>
ResourceFile::ResourceFile(ConversionContext<ecore::Texture2D>& context)
{
	context.get_data(_metadata, _binBlob, _binBlobSize, _path);
}

ResourceFile::ResourceFile(const io::URI& uri)
{
	_path = uri;
}

inline ResourceFile::~ResourceFile()
{
	delete[] _binBlob;
}

void ResourceFile::serialize(uint8_t*& data, uint64_t& size)
{
	int compressStaging = LZ4_compressBound(_binBlobSize);
	std::vector<char> compressedBinBlob;
	compressedBinBlob.resize(compressStaging);

	uint64_t compressedSize = LZ4_compress_default(
		(const char*)_binBlob,
		compressedBinBlob.data(),
		_binBlobSize,
		compressStaging);

	compressedBinBlob.resize(compressedSize);
	
	uint64_t metadataSize = _metadata.size();
	size_t uint64Size = sizeof(uint64_t) * 3;
	size = uint64Size + compressedBinBlob.size() + metadataSize;
	data = new uint8_t[size];
	uint64_t sizes[3] = { metadataSize, compressedSize, _binBlobSize };

	memcpy(data, sizes, uint64Size);
	memcpy(data + uint64Size, _metadata.data(), _metadata.size());
	memcpy(data + uint64Size + _metadata.size(), compressedBinBlob.data(), compressedBinBlob.size());
}

void ResourceFile::deserialize(uint8_t* data, uint64_t size)
{
	uint64_t metaDataSize;
	memcpy(&metaDataSize, data, sizeof(uint64_t));
	data += sizeof(uint64_t);
	uint64_t compressedBlobSize;
	memcpy(&compressedBlobSize, data, sizeof(uint64_t));
	data += sizeof(uint64_t);
	memcpy(&_binBlobSize, data, sizeof(uint64_t));
	data += sizeof(uint64_t);
	_metadata.resize(metaDataSize);
	memcpy(_metadata.data(), data, metaDataSize);
	data += metaDataSize;
	
	if (_binBlobSize != 0)
	{
		uint8_t* compressedBlob = new uint8_t[compressedBlobSize];
		_binBlob = new uint8_t[_binBlobSize];
		memcpy(compressedBlob, data, compressedBlobSize);
		LZ4_decompress_safe((char*)compressedBlob, (char*)_binBlob, compressedBlobSize, _binBlobSize);
		delete[] compressedBlob;
	}
}

inline bool ResourceFile::is_valid()
{
	return _binBlob && !_metadata.empty();
}

inline void ResourceFile::destroy_binary_blob()
{
	delete[] _binBlob;
	_binBlob = nullptr;
}

inline void ResourceFile::accept(IVisitor& visitor)
{
	dynamic_cast<IResourceVisitor&>(visitor).visit(this);
}

LevelFile::LevelFile(const io::URI& uri)
{
	_path = uri;
}
			
LevelFile::~LevelFile()
{
	
}

void LevelFile::serialize(uint8_t*& data, uint64_t& size)
{
	data = new uint8_t[_metadata.size() + sizeof(uint64_t)];
	uint64_t metadataSize = _metadata.size();
	memcpy(data, &metadataSize, sizeof(uint64_t));
	memcpy(data + sizeof(uint64_t), _metadata.data(), metadataSize);
	size = sizeof(uint64_t) + _metadata.size();
}

void LevelFile::deserialize(uint8_t* data, uint64_t size)
{
	uint64_t metadataSize;
	memcpy(&metadataSize, data, sizeof(uint64_t));
	_metadata.resize(metadataSize);
	memcpy(_metadata.data(), data + sizeof(uint64_t), metadataSize);
}

inline bool LevelFile::is_valid()
{
	return !_metadata.empty();
}

inline void LevelFile::accept(IVisitor& visitor)
{
	dynamic_cast<IResourceVisitor&>(visitor).visit(this);
}
