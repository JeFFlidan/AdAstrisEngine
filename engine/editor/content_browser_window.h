#pragma once

#include "common.h"
#include "rhi/ui_window_backend.h"
#include "ui_core/thumbnail_table_manager.h"
#include "ui_core/common.h"
#include "ui_core/file_explorer.h"

namespace ad_astris::editor::impl
{
	class ContentBrowserWindow : public uicore::IUIWindow, public UIWindowInternal
	{
		public:
			ContentBrowserWindow(UIWindowInitContext& initContext, rhi::GetIconsCallback getIconsCallback);
			virtual void draw_window(void* data = nullptr) override;

		private:
			std::unique_ptr<uicore::ThumbnailTableManager> _thumbnailTableManager;
			std::unordered_map<IconType, uicore::TextureInfo> _icons;
			uicore::FileExplorer _fileExplorer;
			uint32_t _thumbnailSize;
			io::URI _currentPath;
			io::URI _baseFolder;
			std::vector<std::string> _allPathDirectories;

			void add_thumbnail(const std::string& label, uicore::TextureInfo textureInfo);
			void build_directories_chain();
	};
}
