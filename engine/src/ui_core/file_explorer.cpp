#include "file_explorer.h"
#include "profiler/logger.h"

using namespace ad_astris::uicore;

FileExplorer::FileExplorer(FileExplorerMode mode) : _fileExplorerMode(mode)
{
	
}

FileExplorer::FileExplorer(FileExplorerMode mode, const std::vector<FileFilter>& fileFilters) : _fileExplorerMode(mode)
{
	for (auto& filter : fileFilters)
	{
		if (filter != FileFilter::ALL)
			parse_one_file_filter_section(filter);
	}

	_filterList.erase(_filterList.find_last_of(';'));

	LOG_INFO("Filter: {}", _filterList);
}

bool FileExplorer::open_file_explorer(std::vector<std::string>& paths, const std::string& defaultPath)
{
	nfdchar_t* defaultPathPtr = defaultPath.empty() ? nullptr : const_cast<char*>(defaultPath.c_str());
	nfdchar_t* filterList = defaultPath.empty() ? nullptr : const_cast<char*>(_filterList.c_str());
	switch (_fileExplorerMode)
	{
		case FileExplorerMode::PICK_FOLDER:
		{
			nfdchar_t* outPath = nullptr;
			nfdresult_t result = NFD_PickFolder(defaultPathPtr, &outPath);
			return check_result(result, paths, outPath);
		}
		case FileExplorerMode::OPEN_FILE:
		{
			nfdchar_t* outPath = nullptr;
			nfdresult_t result = NFD_OpenDialog(filterList, defaultPathPtr, &outPath);
			return check_result(result, paths, outPath);
		}
		case FileExplorerMode::SAVE_FILE:
		{
			nfdchar_t* outPath = nullptr;
			nfdresult_t result = NFD_SaveDialog(filterList, defaultPathPtr, &outPath);
			return check_result(result, paths, outPath);
		}
		case FileExplorerMode::OPEN_MULTIPLE_FILES:
		{
			nfdpathset_t pathSet;
			nfdresult_t result = NFD_OpenDialogMultiple(filterList, defaultPathPtr, &pathSet);
			if (result == NFD_OKAY)
			{
				for (size_t i = 0; i != NFD_PathSet_GetCount(&pathSet); ++i)
				{
					paths.push_back(NFD_PathSet_GetPath(&pathSet, i));
				}
				return true;
			}
			if (result == NFD_CANCEL)
				return false;
			if (result == NFD_ERROR)
			{
				LOG_ERROR("{}", NFD_GetError())
				return false;
			}
		}
	}
}

void FileExplorer::parse_one_file_filter_section(FileFilter enumFileFilter)
{
	if (has_flag(enumFileFilter, FileFilter::AARES))
		_filterList += "aares,";
	if (has_flag(enumFileFilter, FileFilter::AALEVEL))
		_filterList += "aalevel,";
	if (has_flag(enumFileFilter, FileFilter::AAPROJECT))
		_filterList += "aaproject,";
	if (has_flag(enumFileFilter, FileFilter::PNG))
		_filterList += "png,";
	if (has_flag(enumFileFilter, FileFilter::TGA))
		_filterList += "tga,";

	if (_filterList[_filterList.size() - 1] == ',')
	{
		_filterList[_filterList.size() - 1] = ';';
	}
}

bool FileExplorer::check_result(nfdresult_t& result, std::vector<std::string>& paths, nfdchar_t* path)
{
	if (result == NFD_OKAY)
	{
		paths.push_back(path);
		return true;
	}
	if (result == NFD_CANCEL)
		return false;
	if (result == NFD_ERROR)
	{
		LOG_ERROR("{}", NFD_GetError())
		return false;
	}
}
