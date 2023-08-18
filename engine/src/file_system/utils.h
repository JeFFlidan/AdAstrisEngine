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
			static URI get_relative_path_to_file(FileSystem* fileSystem, const URI& pathToFile);
			static URI get_relative_path_to_file(const URI& baseFolder, const URI& pathToFile);
			static URI get_absolute_path_to_file(FileSystem* fileSystem, const URI& relativePath);
			static URI get_absolute_path_to_file(const URI& baseFolder, const URI& relativePath);
			static bool is_absolute(const URI& path);
			static bool is_relative(const URI& path);
			static bool has_extension(const URI& path);
			static bool exists(FileSystem* fileSystem, const URI& path);
			static URI find_file_with_specific_extension(const URI& folderPath, const std::string& extension);
	};
}
