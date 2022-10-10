#include <asset_loader.h>
#include <fstream>
#include <iostream>
#include <cstring>

using namespace assets;
bool assets::save_binaryFile(const char* path, const AssetFile& file)
{
	std::ofstream outFile;
	outFile.open(path, std::ios::binary | std::ios::out);

	outFile.write(file.type, 4);

	uint32_t version = file.version;
	outFile.write((const char*)&version, sizeof(uint32_t));

	uint32_t length = file.json.length();
	outFile.write((const char*)&length, sizeof(uint32_t));

	uint32_t blobLength = file.binaryBlob.size();
	outFile.write((const char*)&blobLength, sizeof(uint32_t));

	outFile.write(file.json.data(), length);
	outFile.write(file.binaryBlob.data(), file.binaryBlob.size());

	outFile.close();

	return true;
}

bool assets::load_binaryFile(const char* path, AssetFile& outputFile)
{
	std::ifstream infile;
	infile.open(path, std::ios::binary);

	if (!infile.is_open()) return false;
	infile.seekg(0);
	infile.read(outputFile.type, 4);
	infile.read((char*)&outputFile.version, sizeof(uint32_t));

	uint32_t jsonLen = 0;
	infile.read((char*)&jsonLen, sizeof(uint32_t));

	uint32_t blobLen = 0;
	infile.read((char*)&blobLen, sizeof(uint32_t));

	outputFile.json.resize(jsonLen);
	infile.read(outputFile.json.data(), jsonLen);
	
	outputFile.binaryBlob.resize(blobLen);
	infile.read(outputFile.binaryBlob.data(), blobLen);

	return true;
}

CompressionMode assets::parse_compression(const char* f)
{
	if (strcmp(f, "LZ4") == 0)
		return CompressionMode::LZ4;
	else
		return CompressionMode::None;
}

bool assets::saveJson(const char* path, AssetFile& file)
{
	std::ofstream out;
	out.open(path);
	uint32_t size = file.json.size();
	out.write(file.json.data(), size);
	out.close();
	return true;
}