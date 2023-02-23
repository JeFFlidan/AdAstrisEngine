#include "IO.h"
#include "profiler/logger.h"
#include <filesystem>
#include <string>

namespace ad_astris
{
	io::EngineFileStream::EngineFileStream(FILE* file)
	{
		this->file = file;
		assert(this->file);
	}

	io::EngineFileStream::~EngineFileStream()
	{
		assert(file);
		fclose(file);
		file = nullptr;
	}

	size_t io::EngineFileStream::read(void* data, size_t size, size_t count)
	{
		assert(file && "EngineFileStream::File is invalid");
		return fread(data, size, count, file);
	}

	size_t io::EngineFileStream::write(const void* data, size_t size, size_t count)
	{
		assert(file && "EngineFileStream::File is invalid");
		return fwrite(data, size, count, file);
	}

	uint64_t io::EngineFileStream::size() const
	{
		assert(file && "EngineFileStream::File is invalid");
		long position = ftell(file);
		fseek(file, 0, SEEK_END);
		long size = ftell(file);
		fseek(file, position, SEEK_SET);
		return static_cast<uint64_t>(size);
	}

	io::EngineFileSystem::EngineFileSystem(const char* path)
	{
		rootPath = std::filesystem::path(path);
	}

	io::EngineFileSystem::~EngineFileSystem() {}

	io::EngineFileStream* io::EngineFileSystem::open(const io::URI& uri, const char* mode)
	{
		std::filesystem::path path = std::filesystem::path(uri.c_str());
		if (!path.is_absolute())
			path = rootPath / path;

		if (!std::filesystem::exists(path))
		{
			LOG_ERROR("Path {} is invlaid", path.string().c_str())
			return nullptr;
		}

		if (!path.has_extension())
		{
			LOG_ERROR("File from path {} has no extension", path.string().c_str())
			return nullptr;
		}
			
		FILE* file = fopen(path.string().c_str(), mode);
		assert(file && "EngineFileSystem::File is invalid after opening");
		return new EngineFileStream(file);
	}

	bool io::EngineFileSystem::close(io::Stream* stream)
	{
		assert(stream);
		delete stream;
		return true;
	}

	void* io::EngineFileSystem::map_to_system(const URI& path, size_t& size, const char* mode)
	{
		EngineFileStream* stream = open(path, mode);
		if (!stream)
		{
			LOG_ERROR("Stream is invalid. Path {}", path.c_str())
			return nullptr;
		}

		size = stream->size();
		if (!size)
		{
			LOG_ERROR("File {} is empty", path.c_str())
			return nullptr;
		}

		uint8_t* data = new uint8_t[size];
		size_t readElements = stream->read(data, sizeof(uint8_t), size);
		assert(readElements == size && ("Data " + std::string(path.c_str()) + " is invalid").c_str());
		assert(data && ("Data " + std::string(path.c_str()) + " is invalid").c_str());

		bool afterClose = close(stream);
		assert(afterClose && "Error while closing stream");

		return data;
	}

	bool io::EngineFileSystem::unmap_from_system(void* data)
	{
		assert(data);
		uint8_t* newData = reinterpret_cast<uint8_t*>(data);
		delete[] newData;
		data = nullptr;

		return true;
	}
}

