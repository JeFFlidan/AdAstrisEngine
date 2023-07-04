#include "utils.h"

#include "algorithm"
#include <filesystem>

using namespace ad_astris;

void io::Utils::replace_back_slash_to_forward(URI& path)
{
	std::string strPath = path.c_str();
	std::replace(strPath.begin(), strPath.end(), '\\', '/');
	path = strPath.c_str();
}

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

io::URI io::Utils::get_relative_path_to_file(FileSystem* fileSystem, const URI& pathToFile)
{
	std::filesystem::path filePath(pathToFile.c_str());
	std::filesystem::path projPath(fileSystem->get_root_path().c_str());
	std::filesystem::path relativePath = std::filesystem::relative(filePath, projPath);
	return relativePath.string().c_str();
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

bool io::Utils::exists(FileSystem* fileSystem, const URI& path)
{
	if (is_absolute(path))
		return std::filesystem::exists(std::filesystem::path(path.c_str()));

	URI newPath = get_absolute_path_to_file(fileSystem, path);
	return std::filesystem::exists(std::filesystem::path(newPath.c_str()));
}
