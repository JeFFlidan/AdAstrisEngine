#include "path_input_widget.h"
#include <imgui.h>

using namespace ad_astris::uicore;

PathInputWidget::PathInputWidget(FileExplorer* fileExplorer, const std::string& defaultPath, bool editable)
	: _fileExplorer(fileExplorer), _textInputWidget("Project Location", 350, defaultPath, editable, 512), _path(defaultPath), _editable(editable)
{
	
}

bool PathInputWidget::draw()
{
	_path = _textInputWidget.draw();
	return true;
}
