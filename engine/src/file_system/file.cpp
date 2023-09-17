#include "file.h"
#include "utils.h"

#include <lz4/lz4.h>

#include <vector>

using namespace ad_astris;
using namespace io;

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
	memcpy(inputDataPtr, &metadataSize, sizeof(uint64_t));
	inputDataPtr += sizeof(uint64_t);
	
	uint64_t compressedBinDataSize = 0;
	memcpy(inputDataPtr, &compressedBinDataSize, sizeof(uint64_t));
	inputDataPtr += sizeof(uint64_t);
	
	uint64_t binDataSize = 0;
	memcpy(inputDataPtr, &binDataSize, sizeof(uint64_t));
	inputDataPtr += sizeof(uint64_t);

	if (metadataSize)
	{
		outputMetadata.resize(metadataSize);
		memcpy(inputDataPtr, outputMetadata.data(), metadataSize);
		inputDataPtr += metadataSize;
	}

	if (compressedBinDataSize && binDataSize)
	{
		std::vector<uint8_t> compressedBin(compressedBinDataSize);
		memcpy(inputDataPtr, compressedBin.data(), compressedBinDataSize);
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
	// TODO
}

void File::deserialize(uint8_t* data, uint64_t size)
{
	// TODO
}

