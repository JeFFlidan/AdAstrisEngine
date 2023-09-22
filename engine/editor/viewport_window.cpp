#include "viewport_window.h"
#include "profiler/logger.h"
#include <imgui/imgui.h>

using namespace ad_astris;
using namespace editor;
using namespace impl;

ViewportWindow::ViewportWindow(std::function<uint64_t()> callback) : _textureCallback(callback)
{

}

void ViewportWindow::draw_window(void* data)
{
	ImGui::Begin("Viewport");
	_textureIndex = _textureCallback();
	ImVec2 viewportExtent = ImGui::GetContentRegionAvail();
	ImGui::Image((void*)_textureIndex, viewportExtent);
	ImGui::End();
}
