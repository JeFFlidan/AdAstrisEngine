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
			ThumbnailTableManager(const std::string& tableName, float thumbnailSize, bool editableButtonLabel);
			void add_button(const std::string& buttonLabel, TextureInfo& textureInfo);
			void draw(std::vector<std::string> selectedButtonNames);

			std::string get_table_name()
			{
				return _tableName;
			}

		private:
			std::unique_ptr<WidgetSelectionManager> _widgetSelectionManager;
			bool _editableButtonLabels;
			uint32_t _thumbnailSize;
			std::string _tableName;
	};
}
