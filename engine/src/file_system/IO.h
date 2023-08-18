#pragma once

#include "file_system.h"
#include "core/pool_allocator.h"
#include <filesystem>

namespace ad_astris::io
{
	class EngineFileStream : public Stream
	{
		public:
			EngineFileStream(FILE* file);
			virtual ~EngineFileStream() final;

			virtual size_t read(void* data, size_t size, size_t count) final;
			virtual size_t write(const void* data, size_t size, size_t count) final;
			virtual uint64_t size() const final;

		private:
			FILE* file;
	};

	class EngineFileSystem : public FileSystem
	{
		public:
			EngineFileSystem(const char* engineRootPath);
			virtual ~EngineFileSystem() final;

			virtual EngineFileStream* open(const URI& path, const char* mode) final;
			virtual bool close(Stream* stream) final;
			virtual void* map_to_read(const URI& uri, size_t& size, const char* mode = "rb") final;
			virtual bool unmap_after_reading(void* data) final;
			virtual void write(const URI& uri, void* data, size_t objectSize, size_t count, const char* mode = "wb") override;

		private:
			ThreadSafePoolAllocator<EngineFileStream> _streamPool;
	};
}
