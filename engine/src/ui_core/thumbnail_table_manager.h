#pragma once

#include "common.h"
#include "widget_selection_manager.h"
#include <string>
#include <vector>
#include <memory>

namespace ad_astris::uicore
{
	class ThumbnailTableManager
	{
		public:
			// Must be used when window with thumbnails is resizable
			ThumbnailTableManager(const std::string& tableName, float thumbnailSize, WidgetSelectionManagerCreationContext& selectionManagerCreationContext);
			void add_button(const std::string& buttonLabel, TextureInfo& textureInfo, bool initiallyActive = false);
			std::unordered_set<std::string>& draw();

			std::string get_table_name()
			{
				return _tableName;
			}

			uint32_t get_thumbnail_size()
			{
				return _thumbnailSize;
			}

		private:
			std::unique_ptr<WidgetSelectionManager> _widgetSelectionManager;
			uint32_t _thumbnailSize;
			std::string _tableName;
	};
}
