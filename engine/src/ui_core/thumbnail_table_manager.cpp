#include "thumbnail_table_manager.h"
#include "profiler/logger.h"

using namespace ad_astris::uicore;

ThumbnailTableManager::ThumbnailTableManager(const std::string& tableName, float thumbnailSize, bool editableButtonLabels)
	: _thumbnailSize(thumbnailSize), _editableButtonLabels(editableButtonLabels), _tableName(tableName)
{
	WidgetSelectionManagerCreationContext creationContext;
	creationContext.multipleSelection = true;
	creationContext.drawImageButtonLabel = true;
	creationContext.editableButtonLabel = _editableButtonLabels;
	_widgetSelectionManager = std::make_unique<WidgetSelectionManager>(creationContext);
}

void ThumbnailTableManager::add_button(const std::string& buttonLabel, TextureInfo& textureInfo)
{
	_widgetSelectionManager->add_image_button(buttonLabel, textureInfo);
}

void ThumbnailTableManager::draw(std::vector<std::string> selectedButtonNames)
{
	float padding = 16.0f;
	float cellSize = _thumbnailSize + padding;
	float panelWidth = ImGui::GetContentRegionAvail().x;
	int32_t columnCount = static_cast<int32_t>(panelWidth / cellSize);
	if (columnCount < 1)
		columnCount = 1;

	ImGui::BeginTable(_tableName.c_str(), columnCount);

	for (uint32_t i = 0; i != _widgetSelectionManager->get_widgets_count(); ++i)
	{
		ImGui::TableNextColumn();
		_widgetSelectionManager->draw_one_widget(i);
	}

	//selectedButtonNames.push_back(_lastSelectionHighlightManager.get_current_selected_button_name());
	ImGui::EndTable();
}
