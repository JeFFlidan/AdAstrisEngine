#pragma once

#include "api.h"
#include "ui_core/text_input_widget.h"
#include "ui_core/thumbnail_table_manager.h"
#include "ui_core/file_explorer.h"
#include "application/project_launcher_module.h"
#include "core/config_base.h"
#include <string>
#include <memory>

namespace ad_astris::devtools::pl_impl
{
	class PROJECT_LAUNCHER_API CreateNewGameBrowser
	{
		public:
			CreateNewGameBrowser(std::string& rootPath, Config* config);

			bool draw_ui();

			ProjectInfo get_new_project_info()
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
			uicore::TextInputWidget _projectNameInputWidget;
			std::unordered_map<std::string, uicore::TextureInfo> _projectTemplateDescriptionTextureByItsName;
			std::unique_ptr<uicore::ThumbnailTableManager> _thumbnailTableManager;
			uicore::FileExplorer _fileExplorer;
			Config* _config{ nullptr };
			bool _cancelButtonPressed{ false };

			void draw_preset_info(std::string& presetName);
			void draw_preset_settings(std::string& presetName);
			void create_folders(std::string& projectPath, std::string& projectName);
			NewProjectTemplate get_enum_project_template_type(std::string& projectTemplateName);
	};
}