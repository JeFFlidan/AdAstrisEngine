#include "docking_window.h"
#include "vk_engine.h"

using namespace engine::ui;

void DockingWindow::draw_window(void* data)
{
	ImGui::SetNextWindowSize(_windowSize);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	windowFlags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoCollapse;
	
	ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
	dockspaceFlags ^= ImGuiDockNodeFlags_PassthruCentralNode;
	
	bool open = true;
		
	ImGui::Begin("Dockspace", &open, windowFlags);
	ImGui::PopStyleVar(3);
		
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspaceID = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
	}
		
	ImGui::End();
}

void DockingWindow::draw_ui(void* data)
{
	// TODO menus
}
