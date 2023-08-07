#include "recent_projects_browser.h"
#include <imgui.h>

using namespace ad_astris::devtools::pl_impl;

RecentProjectsBrowser::RecentProjectsBrowser(std::string& rootPath, Config* config) : _config(config)
{
	
}

bool RecentProjectsBrowser::draw_ui()
{
	ImGui::Begin("Projects");
	ImGui::End();
	ImGui::Begin("Project Info");
	ImGui::End();
	return false;
}
