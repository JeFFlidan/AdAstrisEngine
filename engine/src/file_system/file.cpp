#include "file.h"
#include "utils.h"
#include "profiler/logger.h"

#include <lz4/lz4.h>

#include <vector>

using namespace ad_astris;
using namespace io;

File::~File()
{
	delete[] _binBlob;
}

void File::serialize(
	std::vector<uint8_t>& inputBinData,
	std::string& inputMetadata,
	std::vector<uint8_t>& outputData)
{
	uint64_t metadataSize = inputMetadata.size();
	uint64_t binDataSize = inputBinData.size();
	uint64_t compressedBinDataSize = 0;
	std::vector<uint8_t> compressedBinData;

	if (binDataSize)
	{
		uint64_t compressStaging = binDataSize ? LZ4_compressBound(binDataSize) : 0;
		compressedBinData.resize(compressStaging);
		compressedBinDataSize = LZ4_compress_default(
		   (const char*)inputBinData.data(),
		   (char*)compressedBinData.data(),
		   binDataSize,
		   compressStaging);
		compressedBinData.resize(compressedBinDataSize);
	}

	uint64_t outputDataSize = sizeof(uint64_t) * 3 + metadataSize + compressedBinDataSize;
	outputData.resize(outputDataSize);
	uint8_t* outputDataPtr = outputData.data();
	
	std::vector<uint64_t> sizes = { metadataSize, compressedBinDataSize, binDataSize };
	memcpy(outputDataPtr, sizes.data(), sizeof(uint64_t) * sizes.size());
	outputDataPtr += sizeof(uint64_t) * sizes.size();
	
	if (metadataSize)
	{
		memcpy(outputDataPtr, inputMetadata.c_str(), metadataSize);
		outputDataPtr += metadataSize;
	}

	if (compressedBinDataSize)
	{
		memcpy(outputDataPtr, compressedBinData.data(), compressedBinDataSize);
	}
}

void File::deserialize(
	std::vector<uint8_t>& inputData,
	std::vector<uint8_t>& outputBinData,
	std::string& outputMetadata)
{
	uint8_t* inputDataPtr = inputData.data();
	
	uint64_t metadataSize = 0;
	memcpy(&metadataSize, inputDataPtr, sizeof(uint64_t));
	inputDataPtr += sizeof(uint64_t);
	
	uint64_t compressedBinDataSize = 0;
	memcpy(&compressedBinDataSize, inputDataPtr, sizeof(uint64_t));
	inputDataPtr += sizeof(uint64_t);
	
	uint64_t binDataSize = 0;
	memcpy(&binDataSize, inputDataPtr, sizeof(uint64_t));
	inputDataPtr += sizeof(uint64_t);

	if (metadataSize)
	{
		outputMetadata.resize(metadataSize);
		memcpy(outputMetadata.data(), inputDataPtr, metadataSize);
		inputDataPtr += metadataSize;
	}

	if (compressedBinDataSize && binDataSize)
	{
		std::vector<uint8_t> compressedBin(compressedBinDataSize);
		memcpy(compressedBin.data(), inputDataPtr, compressedBinDataSize);
		outputBinData.resize(binDataSize);
		LZ4_decompress_safe(
			(char*)compressedBin.data(),
			(char*)outputBinData.data(),
			compressedBinDataSize,
			binDataSize);
	}
}

void File::serialize(uint8_t*& data, uint64_t& size)
{
	serialize(&data, &size);
}

void File::serialize(uint8_t** outputData, uint64_t* outputDataSize) const
{
	uint64_t compressedBinDataSize = 0;
	std::vector<uint8_t> compressedBinData;

	if (_binBlob && _binBlobSize)
	{
		uint64_t compressStaging = LZ4_compressBound(_binBlobSize);
		compressedBinData.resize(compressStaging);
		compressedBinDataSize = LZ4_compress_default(
		   (const char*)_binBlob,
		   (char*)compressedBinData.data(),
		   _binBlobSize,
		   compressStaging);
		compressedBinData.resize(compressedBinDataSize);
	}

	*outputDataSize = sizeof(uint64_t) * 3 + _metadata.size() + compressedBinDataSize;
	*outputData = new uint8_t[*outputDataSize];

	uint8_t* tempDataPtr = *outputData;
	
	std::vector<uint64_t> sizes = { _metadata.size(), compressedBinDataSize, _binBlobSize };
	memcpy(tempDataPtr, sizes.data(), sizeof(uint64_t) * sizes.size());
	tempDataPtr += sizeof(uint64_t) * sizes.size();
	
	if (_metadata.size())
	{
		memcpy(tempDataPtr, _metadata.c_str(), _metadata.size());
		tempDataPtr += _metadata.size();
	}

	if (!compressedBinData.empty())
	{
		memcpy(tempDataPtr, compressedBinData.data(), compressedBinDataSize);
	}
}

void File::deserialize(uint8_t* data, uint64_t size)
{
	uint8_t* tempInputDataPtr = data;
	
	uint64_t metadataSize = 0;
	memcpy(&metadataSize, tempInputDataPtr, sizeof(uint64_t));
	tempInputDataPtr += sizeof(uint64_t);
	
	uint64_t compressedBinDataSize = 0;
	memcpy(&compressedBinDataSize, tempInputDataPtr, sizeof(uint64_t));
	tempInputDataPtr += sizeof(uint64_t);
	
	memcpy(&_binBlobSize, tempInputDataPtr, sizeof(uint64_t));
	tempInputDataPtr += sizeof(uint64_t);

	if (metadataSize)
	{
		_metadata.resize(metadataSize);
		memcpy(_metadata.data(), tempInputDataPtr, metadataSize);
		tempInputDataPtr += metadataSize;
	}

	if (compressedBinDataSize && _binBlobSize)
	{
		std::vector<uint8_t> compressedBin(compressedBinDataSize);
		memcpy(compressedBin.data(), tempInputDataPtr, compressedBinDataSize);
		_binBlob = new uint8_t[_binBlobSize];
		LZ4_decompress_safe(
			(char*)compressedBin.data(),
			(char*)_binBlob,
			compressedBinDataSize,
			_binBlobSize);
	}
}

