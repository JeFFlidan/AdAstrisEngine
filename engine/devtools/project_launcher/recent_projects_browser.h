#pragma once

#include "api.h"
#include "application/project_launcher_module.h"
#include "ui_core/text_input_widget.h"
#include "ui_core/thumbnail_table_manager.h"
#include "ui_core/file_explorer.h"
#include "core/config_base.h"
#include <string>
#include <unordered_map>

namespace ad_astris::devtools::pl_impl
{
	class PROJECT_LAUNCHER_API RecentProjectsBrowser
 	{
 		public:
 			RecentProjectsBrowser(Config* config);
 
 			bool draw_ui();

			ProjectInfo get_chosen_project_info() const
 			{
 				return _projectInfo;
 			}

			bool check_cancel_button_pressed()
 			{
 				return _cancelButtonPressed;
 			}
 
 		private:
 			ProjectInfo _projectInfo;
			uicore::TextInputWidget _projectPathInputWidget;
			uicore::TextInputWidget _projectNameWidget;
			std::unordered_map<std::string, std::string> _projectPathByItsName;
			std::unique_ptr<uicore::ThumbnailTableManager> _thumbnailTableManager;
			uicore::FileExplorer _fileExplorer;
			Config* _config{ nullptr };
			bool _cancelButtonPressed{ false };

			void parse_config();
			void add_thumbnail(std::string& projectPath, std::string& buttonName);
			void fill_text_input_widgets(std::unordered_set<std::string>& widgetNames);
 	};
}