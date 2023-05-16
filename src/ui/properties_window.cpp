#include "properties_window.h"
#include "vulkan_renderer/vk_renderer.h"

using namespace ad_astris::ui;

void PropertiesWindow::draw_window(void* data)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
	bool open = true;
	ImGui::Begin("Actor properties", &open, windowFlags);
	draw_ui(data);
	ImGui::End();
}

void PropertiesWindow::draw_ui(void* data)
{
	VkRenderer* engine = static_cast<VkRenderer*>(data);

	if (_actorData == nullptr && _actorType == None)
	{
		_actorData = &engine->_renderScene._pointLights[0];
		_actorType = PointLight;
	}
	
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Select actor"))
		{
			ImGui::MenuItem("Point light", nullptr, &_isPoint);
			if (_isPoint)
			{
				_isSpot = false;
				_isDir = false;
				ImGui::MenuItem("Spot light ", nullptr, &_isSpot);
				ImGui::MenuItem("Dir light  ", nullptr, &_isDir);
				_actorData = &engine->_renderScene._pointLights[0];
				_actorType = PointLight;
			}
			else
			{
				ImGui::MenuItem("Spot light", nullptr, &_isSpot);
				if (_isSpot)
				{
					_isDir = false;
					ImGui::MenuItem("Dir light", nullptr, &_isDir);
					_actorData = &engine->_renderScene._spotLights[0];
					_actorType = SpotLight;
				}
				else
				{
					ImGui::MenuItem("Dir light", nullptr, &_isDir);
					if (_isDir)
					{
						_actorData = &engine->_renderScene._dirLights[0];
						_actorType = DirectionalLight;
					}
				}
			}
	
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (!_isPoint && !_isDir && !_isSpot)
		_actorData = nullptr;

	if (_actorData == nullptr)
	{
		LOG_INFO("Data is nullptr")
		return;
	}

	switch (_actorType)
	{
		case None:
			return;
		case PointLight:
		{
			PointLightUi ui(static_cast<actors::PointLight*>(_actorData), &engine->_renderScene);
			ui.draw_ui();
			break;
		}
		case SpotLight:
		{
			SpotLightUi ui(static_cast<actors::SpotLight*>(_actorData), &engine->_renderScene);
			ui.draw_ui();
			break;
		}
		case DirectionalLight:
		{
			DirLightUi ui(static_cast<actors::DirectionalLight*>(_actorData), &engine->_renderScene);
			ui.draw_ui();
			break;
		}
	}
}
