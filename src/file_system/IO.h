#pragma once

#include "file_system.h"
#include <filesystem>

namespace ad_astris::io
{
	class EngineFileStream : public Stream
	{
		public:
			EngineFileStream(FILE* file);
			~EngineFileStream() final;

			size_t read(void* data, size_t size, size_t count) final;
			size_t write(const void* data, size_t size, size_t count) final;
			uint64_t size() const final;

		private:
			FILE* file;
	};

	class EngineFileSystem : public FileSystem
	{
		public:
			EngineFileSystem(const char* path);
			~EngineFileSystem() final;

			EngineFileStream* open(const URI& path, const char* mode) final;
			bool close(Stream* stream) final;
			void* map_to_read(const URI& uri, size_t& size, const char* mode = "rb") final;
			bool unmap_after_reading(void* data) final;
		private:
			std::filesystem::path rootPath;
	};
}
