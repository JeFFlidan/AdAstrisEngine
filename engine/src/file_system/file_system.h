#pragma once

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <stdint.h>
#include "profiler/logger.h"

namespace ad_astris::io
{
	class URI
	{
		public:
			URI()
			{
				data[0] = '\x0';
			}

			URI(const char* uri)
			{
				size_t len = strlen(uri);
				assert(len < 4096);
				memcpy(data, uri, len);
				data[len] = '\x0';
			}

			URI(const URI& uri)
			{
				memcpy(data, uri.data, sizeof(char) * 4096);
			}

			URI& operator=(const URI& uri)
			{
				memcpy(data, uri.data, sizeof(char) * 4096);
				return *this;
			}

			bool operator==(const URI& uri) const
			{
				return strcmp(data, uri.data) == 0;
			}

			const char* c_str() const { return data; }
			bool empty() const { return data[0] == '\x0'; }

		private:
			char data[4096];
	};

	class Stream
	{
		public:	
			virtual size_t read(void* data, size_t size, size_t count) = 0;
			virtual size_t write(const void* data, size_t size, size_t count) = 0;
			virtual uint64_t size() const = 0;

			virtual ~Stream() {}
	};

	class FileSystem
	{
		public:
			virtual Stream* open(const URI& path, const char* mode) = 0;
			virtual bool close(Stream* stream) = 0;
			virtual void* map_to_read(const URI& uri, size_t& size, const char* mode = "rb") = 0;
			virtual bool unmap_after_reading(void* data) = 0;
			virtual URI get_root_path() = 0;

			virtual ~FileSystem() {}
	};
}
