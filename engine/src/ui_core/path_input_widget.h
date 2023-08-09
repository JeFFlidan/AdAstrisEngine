#pragma once

#include "file_explorer.h"
#include "text_input_widget.h"
#include "internal/base_widget.h"

namespace ad_astris::uicore
{
	class PathInputWidget : public internal::BaseWidget
	{
		public:
			PathInputWidget() = default;
			PathInputWidget(FileExplorer* fileExplorer, const std::string& defaultPath = "", bool editable = true);

			bool draw() override;
			std::string get_path()
			{
				return _path;
			}

		private:
			TextInputWidget _textInputWidget;
			FileExplorer* _fileExplorer{ nullptr };
			std::string _path;
			bool _editable{ true };
	};
}
