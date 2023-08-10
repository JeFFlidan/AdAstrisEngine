#include "thumbnail_table_manager.h"
#include "profiler/logger.h"

using namespace ad_astris::uicore;

ThumbnailTableManager::ThumbnailTableManager(const std::string& tableName, float thumbnailSize, WidgetSelectionManagerCreationContext& selectionManagerCreationContext)
	: _thumbnailSize(thumbnailSize), _tableName(tableName)
{
	_widgetSelectionManager = std::make_unique<WidgetSelectionManager>(selectionManagerCreationContext);
}

void ThumbnailTableManager::add_button(const std::string& buttonLabel, TextureInfo& textureInfo, bool initiallyActive)
{
	_widgetSelectionManager->add_image_button(buttonLabel, textureInfo, initiallyActive);
}

std::unordered_set<std::string>& ThumbnailTableManager::draw()
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
	
	ImGui::EndTable();

	return _widgetSelectionManager->get_current_selected_widget_names();
}
