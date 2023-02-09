#include "proj_settings_window.h"
#include "vulkan_renderer/vk_renderer.h"

using namespace engine::ui;

void ProjSettingsWindow::draw_window(void* data)
{
	ImGui::Begin("Settings");
	draw_ui(data);
	ImGui::End();
}

void ProjSettingsWindow::draw_ui(void* data)
{
	// TODO settings for project
	Settings* settings = static_cast<Settings*>(data);
	ImGui::Checkbox("TAA", reinterpret_cast<bool*>(&settings->isTaaEnabled));
	if (settings->isTaaEnabled)
		ImGui::SliderFloat("TAA alpha", &settings->taaAlpha, 1.0f, 80.0f);
}
