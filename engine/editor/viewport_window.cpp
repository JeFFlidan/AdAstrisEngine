#include "viewport_window.h"
#include "application_core/editor_events.h"
#include "profiler/logger.h"
#include <imgui/imgui.h>

using namespace ad_astris;
using namespace editor;
using namespace impl;

ViewportWindow::ViewportWindow(UIWindowInitContext& initContext, std::function<uint64_t()> callback) : UIWindowInternal(initContext), _textureCallback(callback)
{

}

void ViewportWindow::draw_window(void* data)
{
	ImGui::Begin("Viewport");
	_textureIndex = _textureCallback();
	ImVec2 newViewportExtent = ImGui::GetContentRegionAvail();
	if (newViewportExtent.x != _viewportExtent.x || newViewportExtent.y != _viewportExtent.y)
	{
		_viewportExtent = newViewportExtent;
		acore::ViewportResizedEvent event(_viewportExtent.x, _viewportExtent.y);
		_eventManager->enqueue_event(event);
	}
	ImGui::Image((void*)_textureIndex, _viewportExtent);

	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    ImVec2 vMax = ImGui::GetWindowContentRegionMax();

    vMin.x += ImGui::GetWindowPos().x;
    vMin.y += ImGui::GetWindowPos().y;
    vMax.x += ImGui::GetWindowPos().x;
    vMax.y += ImGui::GetWindowPos().y;

	acore::ViewportState viewportState;
	viewportState.isHovered = ImGui::IsItemHovered();
	viewportState.viewportMin = XMFLOAT2(vMin.x, vMin.y);
	viewportState.viewportMax = XMFLOAT2(vMax.x, vMax.y);

	acore::ViewportHoverEvent event(viewportState);
	_eventManager->enqueue_event(event);
	
	ImGui::End();
}
