#pragma once

#include "file_system.h"

#include <string>

namespace ad_astris::io
{
	class Utils
	{
		public:
			static void replace_back_slash_to_forward(URI& path);
			static void replace_filename(URI& path, const std::string& newFileName);
			static std::string get_file_name(const URI& path);
			static std::string get_file_extension(const URI& path);

			// Returns the path relative to engine source root path. Not recommended to use anywhere other than engine internal code
			static URI get_relative_path_to_file(FileSystem* fileSystem, const URI& pathToFile);
			// Returns the path relative to the baseFolder value.
			static URI get_relative_path_to_file(const URI& baseFolder, const URI& pathToFile);
			// Makes an absolute path based on the engine source root path. Not recommended to use anywhere other than engine internal code
			static URI get_absolute_path_to_file(FileSystem* fileSystem, const URI& relativePath);
			// Makes an absolute path based on the baseFolder value 
			static URI get_absolute_path_to_file(const URI& baseFolder, const URI& relativePath);
			static bool is_absolute(const URI& path);
			static bool is_relative(const URI& path);
			static bool has_extension(const URI& path);
			static bool exists(const URI& absolutePath);
			static bool exists(const URI& baseFolder, const URI& relativePath);
			// Checks if file exists in the engine sources. Not recommended to use anywhere other than engine internal code
			static bool exists(FileSystem* fileSystem, const URI& path);
			static URI find_file_with_specific_extension(const URI& folderPath, const std::string& extension);

			/**
			 * \brief Reads file data and stores it into a vector
			 * \param fileSystem must be a valid pointer to the FileSystem
			 * \param path must be an absolute path to the file. NO RELATIVE PATH!!!
			 * \param dataStorage is an empty vector that will be resized in function. New vector size = file size in bytes
			 */
			static void read_file(FileSystem* fileSystem, const URI& path, std::vector<uint8_t>& dataStorage);
			static void read_file(FileSystem* fileSystem, const URI& path, uint8_t** dataStorage);
			static void write_file(FileSystem* fileSystem, const URI& path, const uint8_t* data, size_t dataSize, const std::string& writeMode = "wb");
			static void write_file(FileSystem* fileSystem, const URI& path, const char* data, size_t dataSize, const std::string& writeMode = "wb");
			static void serialize_file(FileSystem* fileSystem, const URI& path, std::vector<uint8_t>& inputBinData, std::string& inputMetadata);
			static void deserialize_file(FileSystem* fileSystem, const URI& path, std::vector<uint8_t>& outputBinData, std::string& outputMetadata);
			static uint64_t get_last_write_time(const URI& absolutePath);
	};
}
