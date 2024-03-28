#pragma once

#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstring>
#include <cassert>
#include <stdint.h>

namespace ad_astris::io
{
	constexpr uint64_t MAX_PATH_LENGTH = 4096;
	
	class URI
	{
		public:
			URI() = default;
		
			URI(const char* uri)
			{
				size_t len = strlen(uri);
				assert(len < 2048);
				data = uri;
			}

			URI(const std::string& uri)
			{
				assert(uri.size() < MAX_PATH_LENGTH);
				data = uri;
			}

			URI(const URI& uri)
			{
				assert(uri.data.size() < MAX_PATH_LENGTH);
				data = uri.data;
			}

			URI& operator=(const URI& uri)
			{
				assert(uri.data.size() < MAX_PATH_LENGTH);
				data = uri.data;
				return *this;
			}

			bool operator==(const URI& uri) const
			{
				return data == uri.data;
			}

			URI operator+(const URI& uri) const
			{
				assert(uri.data.size() < MAX_PATH_LENGTH);
				return data + uri.data;
			}

			URI& operator+=(const URI& uri)
			{
				assert(uri.data.size() < MAX_PATH_LENGTH);
				data += uri.data;
				return *this;
			}

			const std::string& string() const { return data; }
			const char* c_str() const { return data.c_str(); }
			bool empty() { return data.empty(); }

		private:
			std::string data;
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
			virtual void write(const URI& uri, void* data, size_t objectSize, size_t count, const char* mode = "wb") = 0;
		
			URI get_engine_root_path()
			{
				return _engineRootPath.string().c_str();
			}
		
			void set_project_root_path(const URI& projectRootPath)
			{
				_projectRootPath = projectRootPath.c_str();
			}
		
			URI get_project_root_path()
			{
				return _projectRootPath.string().c_str(); 
			}

			virtual ~FileSystem() {}

		protected:
			std::filesystem::path _engineRootPath;
			std::filesystem::path _projectRootPath;
	};
}

