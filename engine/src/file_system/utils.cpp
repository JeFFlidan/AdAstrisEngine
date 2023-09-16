#include "utils.h"
#include "profiler/logger.h"

#include <algorithm>
#include <filesystem>

using namespace ad_astris;

void io::Utils::replace_back_slash_to_forward(URI& path)
{
	std::string strPath = path.c_str();
	std::replace(strPath.begin(), strPath.end(), '\\', '/');
	path = strPath.c_str();
}

void io::Utils::replace_filename(URI& path, const std::string& newFileName)
{
	std::filesystem::path stdPath(path.c_str());
	if (stdPath.has_extension())
	{
		std::string fileNameWithExtension = newFileName + '.' + get_file_extension(path);
		path = stdPath.replace_filename(fileNameWithExtension).string().c_str();
	}
	else if (std::filesystem::path(newFileName).has_extension())
	{
		path = stdPath.replace_filename(newFileName).string().c_str();
	}
	else
	{
		LOG_ERROR("io::Utils::replace_filename(): Can't change filename when extension is undefined")
	}
}

std::string io::Utils::get_file_name(const URI& path)
{
	std::string strPath = std::filesystem::path(path.c_str()).filename().string();
	if (strPath.find(".") != std::string::npos)
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
	std::filesystem::path projPath(fileSystem->get_engine_root_path().c_str());
	std::filesystem::path relativePath = std::filesystem::relative(filePath, projPath);
	return relativePath.string().c_str();
}

io::URI io::Utils::get_relative_path_to_file(const URI& baseFolder, const URI& pathToFile)
{
	std::filesystem::path filePath(pathToFile.c_str());
	std::filesystem::path projPath(baseFolder.c_str());
	std::filesystem::path relativePath = std::filesystem::relative(filePath, projPath);
	return relativePath.string().c_str();
}

io::URI io::Utils::get_absolute_path_to_file(FileSystem* fileSystem, const URI& relativePath)
{
	std::filesystem::path cppPath(relativePath.c_str());
	std::filesystem::path rootPath(fileSystem->get_engine_root_path().c_str());
	std::filesystem::path absolutePath = rootPath / cppPath; 
	return URI(absolutePath.string().c_str());
}

io::URI io::Utils::get_absolute_path_to_file(const URI& baseFolder, const URI& relativePath)
{
	std::filesystem::path cppPath(relativePath.c_str());
	std::filesystem::path rootPath(baseFolder.c_str());
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

bool io::Utils::has_extension(const URI& path)
{
	return std::filesystem::path(path.c_str()).has_extension();
}

bool io::Utils::exists(const URI& baseFolder, const URI& relativePath)
{
	URI newPath = get_absolute_path_to_file(baseFolder, relativePath);
	return std::filesystem::exists(std::filesystem::path(newPath.c_str()));
}

bool io::Utils::exists(FileSystem* fileSystem, const URI& path)
{
	if (is_absolute(path))
		return std::filesystem::exists(std::filesystem::path(path.c_str()));

	URI newPath = get_absolute_path_to_file(fileSystem, path);
	return std::filesystem::exists(std::filesystem::path(newPath.c_str()));
}

io::URI io::Utils::find_file_with_specific_extension(const URI& folderPath, const std::string& extension)
{
	for (const auto& entry : std::filesystem::directory_iterator(folderPath.c_str()))
	{
		if (std::filesystem::is_regular_file(entry) && entry.path().extension() == extension)
			return entry.path().string().c_str();
	}

	return URI();
}

void io::Utils::read_file(FileSystem* fileSystem, const URI& path, std::vector<uint8_t>& dataStorage)
{
	size_t size = 0;
	uint8_t* tempData = static_cast<uint8_t*>(fileSystem->map_to_read(path, size));
	dataStorage.resize(size);
	memcpy(dataStorage.data(), tempData, size);
	fileSystem->unmap_after_reading(tempData);
}

void io::Utils::write_file(FileSystem* fileSystem, const URI& path, uint8_t* data, size_t dataSize, const std::string& writeMode)
{
	Stream* stream = fileSystem->open(path, writeMode.c_str());
	stream->write(data, sizeof(uint8_t), dataSize);
	fileSystem->close(stream);
}
