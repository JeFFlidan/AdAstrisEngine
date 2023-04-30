#include "utils.h"

#include <filesystem>

using namespace ad_astris;

std::string io::Utils::get_file_name(const URI& path)
{
	std::string strPath = std::filesystem::path(path.c_str()).filename().string();
	strPath.erase(strPath.find("."), strPath.size());
	return strPath;
}

std::string io::Utils::get_file_extension(const URI& path)
{
	std::string extension = std::filesystem::path(path.c_str()).extension().string();
	extension.erase(0, 1);
	return extension;
}

io::URI io::Utils::get_absolute_path_to_file(FileSystem* fileSystem, const URI& relativePath)
{
	std::filesystem::path cppPath(relativePath.c_str());
	std::filesystem::path rootPath(fileSystem->get_root_path().c_str());
	std::filesystem::path absolutePath = rootPath / cppPath; 
	return URI(absolutePath.string().c_str());
}

bool io::Utils::is_absolute(const URI& path)
{
	return std::filesystem::path(path.c_str()).is_absolute();
}

bool io::Utils::is_relative(const URI& path)
{
	return std::filesystem::path(path.c_str()).is_relative();
}
