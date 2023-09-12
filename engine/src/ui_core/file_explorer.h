#pragma once

#include "core/flags_operations.h"
#include <nfd/nfd.h>
#include <vector>
#include <string>

namespace ad_astris::uicore
{
	enum class FileExplorerMode
	{
		OPEN_FILE,
		SAVE_FILE,
		OPEN_MULTIPLE_FILES,
		PICK_FOLDER
	};

	// TODO Add more file types
	enum class FileFilter
	{
		ALL = 0,
		AARES = 1ULL << 0,
		AALEVEL = 1ULL << 1,
		AAPROJECT = 1ULL << 2,
		PNG = 1ULL << 3,
		TGA = 1ULL << 4,
	};
	
	class FileExplorer
	{
		public:
			FileExplorer() = default;
			// No filter. Should be used with PICK_FOLDER mode or when you don't want to have file filter when opening a file
			FileExplorer(FileExplorerMode mode);
			/** Should be used with one of those modes: OPEN_FILE, SAVE_FILE or OPEN_MULTIPLE_FILE.
			Every FileFilter object in a vector describes one filter section (for example, filter for png, jpg
			and another filter for fbx, obj.) if FileExplorerMode is OPEN_FILE or OPEN_MULTIPLE_FILES.
			If FileExplorerMode is SAVE_FILE, every FileFilter in a vector must contain only one file type*/
			FileExplorer(FileExplorerMode mode, const std::vector<FileFilter>& fileFilters);
			bool open_file_explorer(std::vector<std::string>& paths, const std::string& defaultPat = "");

		private:
			FileExplorerMode _fileExplorerMode;
			std::string _filterList;

			void parse_one_file_filter_section(FileFilter enumFileFilter);
			bool check_result(nfdresult_t& result, std::vector<std::string>& paths, nfdchar_t* path);
	};
}

template<>
struct EnableBitMaskOperator<ad_astris::uicore::FileFilter>
{
	static const bool enable = true;
};

