#include "file.h"
#include "utils.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"
#include "profiler/logger.h"

#include <lz4.h>

#include <vector>

using namespace ad_astris;

void io::IFile::set_metadata(std::string& newMetaData)
{
	_metaData = newMetaData;
}

std::string& io::IFile::get_metadata()
{
	return _metaData;
}

io::URI io::IFile::get_file_path()
{
	return _path;
}

std::string io::IFile::get_file_name()
{
	return Utils::get_file_name(_path);
}

template<typename T>
io::ResourceFile::ResourceFile(ConversionContext<T>& context)
{
	
}

template<>
io::ResourceFile::ResourceFile(ConversionContext<ecore::StaticModel>& context)
{
	context.get_data(_metaData, _binBlob, _binBlobSize, _path);
}

template<>
io::ResourceFile::ResourceFile(ConversionContext<ecore::Texture2D>& context)
{
	context.get_data(_metaData, _binBlob, _binBlobSize, _path);
}

io::ResourceFile::ResourceFile(const URI& uri)
{
	_path = uri;
}

io::ResourceFile::~ResourceFile()
{
	delete[] _binBlob;
}

void io::ResourceFile::serialize(uint8_t*& data, uint64_t& size)
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
	
	uint64_t metadataSize = _metaData.size();
	size_t uint64Size = sizeof(uint64_t) * 3;
	size = uint64Size + compressedBinBlob.size() + metadataSize;
	data = new uint8_t[size];
	uint64_t sizes[3] = { metadataSize, compressedSize, _binBlobSize };

	memcpy(data, sizes, uint64Size);
	memcpy(data + uint64Size, _metaData.data(), _metaData.size());
	memcpy(data + uint64Size + _metaData.size(), compressedBinBlob.data(), compressedBinBlob.size());
}

void io::ResourceFile::deserialize(uint8_t* data, uint64_t size)
{
	uint64_t metaDataSize;
	memcpy(&metaDataSize, data, sizeof(uint64_t));
	data += sizeof(uint64_t);
	uint64_t compressedBlobSize;
	memcpy(&compressedBlobSize, data, sizeof(uint64_t));
	data += sizeof(uint64_t);
	memcpy(&_binBlobSize, data, sizeof(uint64_t));
	data += sizeof(uint64_t);
	_metaData.resize(metaDataSize);
	memcpy(_metaData.data(), data, metaDataSize);
	data += metaDataSize;
	uint8_t* compressedBlob = new uint8_t[compressedBlobSize];
	_binBlob = new uint8_t[_binBlobSize];
	memcpy(compressedBlob, data, compressedBlobSize);
	LZ4_decompress_safe((char*)compressedBlob, (char*)_binBlob, compressedBlobSize, _binBlobSize);
	delete[] compressedBlob;
}

bool io::ResourceFile::is_valid()
{
	return _binBlob && !_metaData.empty();
}


uint8_t* io::ResourceFile::get_binary_blob()
{
	return _binBlob;
}

uint64_t io::ResourceFile::get_binary_blob_size()
{
	return _binBlobSize;
}

io::LevelFile::LevelFile(const URI& uri)
{
	_path = uri;
}
			
io::LevelFile::~LevelFile()
{
	
}

void io::LevelFile::serialize(uint8_t*& data, uint64_t& size)
{
	data = new uint8_t[_metaData.size() + sizeof(uint64_t)];
	LOG_INFO("Meta data in file serialize: {}", _metaData)
	LOG_INFO("Array size: {}", _metaData.size() + sizeof(uint64_t))
	uint64_t metadataSize = _metaData.size();
	memcpy(data, &metadataSize, sizeof(uint64_t));
	memcpy(data + sizeof(uint64_t), _metaData.data(), metadataSize);
	size = sizeof(uint64_t) + _metaData.size();
}

void io::LevelFile::deserialize(uint8_t* data, uint64_t size)
{
	uint64_t metadataSize;
	memcpy(&metadataSize, data, sizeof(uint64_t));
	_metaData.resize(metadataSize);
	memcpy(_metaData.data(), data + sizeof(uint64_t), metadataSize);
}

bool io::LevelFile::is_valid()
{
	return !_metaData.empty();
}

uint8_t* io::LevelFile::get_binary_blob()
{
	return nullptr;
}

uint64_t io::LevelFile::get_binary_blob_size()
{
	return 0;
}
