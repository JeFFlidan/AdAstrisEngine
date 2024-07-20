#pragma once

#include "common.h"
#include "ui_core/thumbnail_table_manager.h"
#include "ui_core/common.h"
#include "ui_core/file_explorer.h"

namespace ad_astris::editor::impl
{
	class ContentBrowserWindow : public uicore::IUIWindow, public UIWindowInternal
	{
		public:
			ContentBrowserWindow(UIWindowInitContext& initContext);
			virtual void draw_window(void* data = nullptr) override;

		private:
			enum IconType
			{
				FOLDER,
				TEXTURE_FILE,
				MODEL_FILE,
				LEVEL_FILE,
				MATERIAL_FILE,

				ICON_COUNT
			};
		
			std::unique_ptr<uicore::ThumbnailTableManager> _thumbnailTableManager;
			uicore::TextureInfo _icons[ICON_COUNT];
			uicore::FileExplorer _fileExplorer;
			uint32_t _thumbnailSize;
			io::URI _currentPath;
			io::URI _baseFolder;
			std::vector<std::string> _allPathDirectories;

			void add_thumbnail(const std::string& label, uicore::TextureInfo textureInfo);
			void build_directories_chain();
	};
}
