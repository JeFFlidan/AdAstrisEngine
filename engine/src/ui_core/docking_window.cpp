#include "docking_window.h"

using namespace ad_astris::uicore;

void DockingWindow::draw_window(void* data)
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;
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
