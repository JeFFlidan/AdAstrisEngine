#include "actors_properties_ui.h"
#include "glm/vec3.hpp"
#include "glm/trigonometric.hpp"

using namespace engine::ui;

PointLightUi::PointLightUi(actors::PointLight* pointLight, RenderScene* renderScene)
	: _pointLight(pointLight), _renderScene(renderScene)
{
	
}

void PointLightUi::draw_ui()
{
	if (ImGui::CollapsingHeader("Transform"))
	{
		float position[3] = {
			_pointLight->positionAndAttRadius.x,
			_pointLight->positionAndAttRadius.y,
			_pointLight->positionAndAttRadius.z };
		
		bool check = ImGui::DragFloat3("Translation", position, 0.1f, -100000.0f, 100000.0f);
		
		_pointLight->positionAndAttRadius.x = position[0];
		_pointLight->positionAndAttRadius.y = position[1];
		_pointLight->positionAndAttRadius.z = position[2];
		
		_renderScene->_bNeedsBakePointShadows.push_back(check);
		_renderScene->_bNeedsReloadingPointLights.push_back(check);
	}
	if (ImGui::CollapsingHeader("Light"))
	{
		bool check = ImGui::SliderFloat("Source radius", &_pointLight->sourceRadius, 0.0f, 1500.0f);
		_renderScene->_bNeedsReloadingPointLights.push_back(check);
		check = ImGui::SliderFloat("Attenuation radius", &_pointLight->positionAndAttRadius.w, 0.0f, 1500.0f);
		_renderScene->_bNeedsReloadingPointLights.push_back(check);
		check = ImGui::Checkbox("Cast shadows", reinterpret_cast<bool*>(&_pointLight->castShadows));
		_renderScene->_bNeedsReloadingPointLights.push_back(check);

		float color[3] = {
			_pointLight->colorAndIntensity.x,
			_pointLight->colorAndIntensity.y,
			_pointLight->colorAndIntensity.z };

		check = ImGui::ColorEdit3("Light color", color);

		_pointLight->colorAndIntensity.x = color[0];
		_pointLight->colorAndIntensity.y = color[1];
		_pointLight->colorAndIntensity.z = color[2];
		
		_renderScene->_bNeedsReloadingPointLights.push_back(check);
		check = ImGui::SliderFloat("Intensity (lm)", &_pointLight->colorAndIntensity.w, 0.0f, 100000.0f);
		_renderScene->_bNeedsReloadingPointLights.push_back(check);
	}
	if (ImGui::CollapsingHeader("Rendering"))
	{
		bool check = ImGui::Checkbox("Visible", reinterpret_cast<bool*>(&_pointLight->isVisible));
		_renderScene->_bNeedsReloadingPointLights.push_back(check);
	}
}

SpotLightUi::SpotLightUi(actors::SpotLight* spotLight, RenderScene* renderScene)
	: _spotLight(spotLight), _renderScene(renderScene)
{
	
}

void SpotLightUi::draw_ui()
{
	if (ImGui::CollapsingHeader("Transform"))
	{
		float translation[3] = {
			_spotLight->positionAndDistance.x,
			_spotLight->positionAndDistance.y,
			_spotLight->positionAndDistance.z };
		
		bool check = ImGui::DragFloat3("Translation", translation, 0.1f, -100000.0f, 100000.0f);

		_spotLight->positionAndDistance.x = translation[0];
		_spotLight->positionAndDistance.y = translation[1];
		_spotLight->positionAndDistance.z = translation[2];
		
		_renderScene->_bNeedsBakeSpotShadows.push_back(check);
		_renderScene->_bNeedsReloadingSpotLights.push_back(check);

		float rotation[3] = {
			glm::degrees(_spotLight->rotationAndInnerConeRadius.x),
			glm::degrees(_spotLight->rotationAndInnerConeRadius.y),
			glm::degrees(_spotLight->rotationAndInnerConeRadius.z),
		};
		
		check = ImGui::DragFloat3("Rotation", rotation, 0.25f, -720.0f, 720.0f);

		_spotLight->rotationAndInnerConeRadius.x = glm::radians(rotation[0]);
		_spotLight->rotationAndInnerConeRadius.y = glm::radians(rotation[1]);
		_spotLight->rotationAndInnerConeRadius.z = glm::radians(rotation[2]);
		
		_renderScene->_bNeedsBakeSpotShadows.push_back(check);
		_renderScene->_bNeedsReloadingSpotLights.push_back(check);
	}
	if (ImGui::CollapsingHeader("Light"))
	{
		float innerConeRadius = glm::degrees(glm::acos(_spotLight->rotationAndInnerConeRadius.w));
		bool check = ImGui::SliderFloat("Inner cone angle", &innerConeRadius, 0.0f, 80.0f);
		_spotLight->rotationAndInnerConeRadius.w = glm::cos(glm::radians(innerConeRadius));
		_renderScene->_bNeedsReloadingSpotLights.push_back(check);

		float outerConeRadius = glm::degrees(glm::acos(_spotLight->outerConeRadius));
		check = ImGui::SliderFloat("Outer cone angle", &outerConeRadius, 0.0f, 80.0f);
		_spotLight->outerConeRadius = glm::cos(glm::radians(outerConeRadius));
		_renderScene->_bNeedsReloadingSpotLights.push_back(check);
		
		check = ImGui::SliderFloat("Distance", &_spotLight->positionAndDistance.w, 10.0f, 1000.0f);
		_renderScene->_bNeedsReloadingSpotLights.push_back(check);
		check = ImGui::Checkbox("Cast shadows", reinterpret_cast<bool*>(&_spotLight->castShadows));
		_renderScene->_bNeedsReloadingSpotLights.push_back(check);

		float color[3] = {
			_spotLight->colorAndIntensity.x,
			_spotLight->colorAndIntensity.y,
			_spotLight->colorAndIntensity.z,
		};
		
		check = ImGui::ColorEdit3("Light color", color);

		_spotLight->colorAndIntensity.x = color[0];
		_spotLight->colorAndIntensity.y = color[1];
		_spotLight->colorAndIntensity.z = color[2];
		
		_renderScene->_bNeedsReloadingSpotLights.push_back(check);
		check = ImGui::SliderFloat("Intensity (lm)", &_spotLight->colorAndIntensity.w, 0.0f, 100000.0f);
		_renderScene->_bNeedsReloadingSpotLights.push_back(check);
	}
	if (ImGui::CollapsingHeader("Rendering"))
	{
		bool check = ImGui::Checkbox("Visible", reinterpret_cast<bool*>(&_spotLight->isVisible));
		_renderScene->_bNeedsReloadingSpotLights.push_back(check);
	}
}

DirLightUi::DirLightUi(actors::DirectionalLight* dirLight, RenderScene* renderScene)
	: _dirLight(dirLight), _renderScene(renderScene)
{
	
}

void DirLightUi::draw_ui()
{
	if (ImGui::CollapsingHeader("Light"))
	{
		bool check = ImGui::Checkbox("Cast shadows", reinterpret_cast<bool*>(&_dirLight->castShadows));
		_renderScene->_bNeedsRealoadingDirLights.push_back(check);

		float color[3] = {
			_dirLight->colorAndIntensity.x,
			_dirLight->colorAndIntensity.y,
			_dirLight->colorAndIntensity.z
		};
		
		check = ImGui::ColorEdit3("Light color", color);

		_dirLight->colorAndIntensity.x = color[0];
		_dirLight->colorAndIntensity.y = color[1];
		_dirLight->colorAndIntensity.z = color[2];
		
		_renderScene->_bNeedsRealoadingDirLights.push_back(check);
		check = ImGui::SliderFloat("Intensity (cd)", &_dirLight->colorAndIntensity.w, 0.0f, 1500.0f);
		_renderScene->_bNeedsRealoadingDirLights.push_back(check);
	}

	if (ImGui::CollapsingHeader("Rendering"))
	{
		bool check = ImGui::Checkbox("Visible", reinterpret_cast<bool*>(&_dirLight->isVisible));
		_renderScene->_bNeedsRealoadingDirLights.push_back(check);
	}
}
