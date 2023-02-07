#include "glm/trigonometric.hpp"
#include "imgui.h"
#include "material_system.h"
#include "vk_engine.h"
#include "user_interface.h"

#include <set>

#include "engine_actors.h"
#include "vk_renderpass.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vulkan/vulkan_core.h>

namespace ui
{
	void UserInterface::init_ui(VulkanEngine* engine)
	{
		VkDescriptorPoolSize pool_sizes[] = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		VkDescriptorPool imguiPool;
		if(vkCreateDescriptorPool(engine->_device, &pool_info, nullptr, &imguiPool) != VK_SUCCESS)
		{
			LOG_ERROR("Failed to create descriptor pool for imgui");
			return;
		}

		ImGui::CreateContext();
		ImGui_ImplSDL2_InitForVulkan(engine->_window);

		ImGui_ImplVulkan_InitInfo init_info{};
		init_info.Instance = engine->_instance;
		init_info.PhysicalDevice = engine->_chosenGPU;
		init_info.Device = engine->_device;
		init_info.Queue = engine->_graphicsQueue;
		init_info.DescriptorPool = imguiPool;
		init_info.MinImageCount = 3;
		init_info.ImageCount = 3;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&init_info, engine->_renderPass);

		engine->immediate_submit([&](VkCommandBuffer cmd) {
			ImGui_ImplVulkan_CreateFontsTexture(cmd);
		});

		ImGui_ImplVulkan_DestroyFontUploadObjects();
		
		engine->_mainDeletionQueue.push_function([=](){
			vkDestroyDescriptorPool(engine->_device, imguiPool, nullptr);
			ImGui_ImplVulkan_Shutdown();
		});

		setup_dark_theme();
	}

	void UserInterface::setup_dark_theme()
	{
		// Taked from https://github.com/ocornut/imgui/issues/707
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg]                = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
		colors[ImGuiCol_Border]                 = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
		colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
		colors[ImGuiCol_FrameBg]                = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
		colors[ImGuiCol_FrameBgActive]          = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colors[ImGuiCol_TitleBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TitleBgActive]          = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
		colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		colors[ImGuiCol_CheckMark]              = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_SliderGrab]             = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		colors[ImGuiCol_Button]                 = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colors[ImGuiCol_ButtonHovered]          = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
		colors[ImGuiCol_ButtonActive]           = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colors[ImGuiCol_Header]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_HeaderHovered]          = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
		colors[ImGuiCol_HeaderActive]           = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
		colors[ImGuiCol_Separator]              = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		colors[ImGuiCol_SeparatorActive]        = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_ResizeGrip]             = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_Tab]                    = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TabHovered]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
		colors[ImGuiCol_TabUnfocused]           = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		//colors[ImGuiCol_DockingPreview]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		//colors[ImGuiCol_DockingEmptyBg]         = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram]          = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TableBorderLight]       = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colors[ImGuiCol_DragDropTarget]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_NavHighlight]           = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding                     = ImVec2(8.00f, 8.00f);
		style.FramePadding                      = ImVec2(5.00f, 2.00f);
		style.CellPadding                       = ImVec2(6.00f, 6.00f);
		style.ItemSpacing                       = ImVec2(6.00f, 6.00f);
		style.ItemInnerSpacing                  = ImVec2(6.00f, 6.00f);
		style.TouchExtraPadding                 = ImVec2(0.00f, 0.00f);
		style.IndentSpacing                     = 25;
		style.ScrollbarSize                     = 15;
		style.GrabMinSize                       = 10;
		style.WindowBorderSize                  = 1;
		style.ChildBorderSize                   = 1;
		style.PopupBorderSize                   = 1;
		style.FrameBorderSize                   = 1;
		style.TabBorderSize                     = 1;
		style.WindowRounding                    = 7;
		style.ChildRounding                     = 4;
		style.FrameRounding                     = 3;
		style.PopupRounding                     = 4;
		style.ScrollbarRounding                 = 9;
		style.GrabRounding                      = 3;
		style.LogSliderDeadzone                 = 4;
		style.TabRounding                       = 4;
	}

	void UserInterface::draw_ui(VulkanEngine* engine)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame(engine->_window);

		ImGui::NewFrame();

		actors::PointLight& pointLight = engine->_renderScene._pointLights[0];
		actors::SpotLight& spotLight = engine->_renderScene._spotLights[0];
		actors::DirectionLight& dirLight = engine->_renderScene._dirLights[0];

		PointLightWindow pointLightWindow(pointLight);
		SpotLightWindow spotLightWindow(spotLight);
		DirLightWindow dirLightWindow(dirLight);
		SettingsWindow settingsWindow(engine->_settings);
		
		pointLightWindow.draw_window(engine);
		spotLightWindow.draw_window(engine);
		dirLightWindow.draw_window(engine);
		settingsWindow.draw_window();
		
		ImGui::EndFrame();

		pointLightWindow.set_point_light_data();
		spotLightWindow.set_spot_light_data();
		dirLightWindow.set_dir_light_data();
		settingsWindow.set_settings_data();
	}

	void UserInterface::render_ui(VulkanEngine* engine)
	{
		//ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), engine->get_current_frame()._mainCommandBuffer);
	}

	PointLightWindow::PointLightWindow(actors::PointLight& pointLight)
	{
		_position[0] = pointLight.positionAndAttRadius.x;
		_position[1] = pointLight.positionAndAttRadius.y;
		_position[2] = pointLight.positionAndAttRadius.z;
		_attenuationRadius = pointLight.positionAndAttRadius.w;

		_color[0] = pointLight.colorAndIntensity.x;
		_color[1] = pointLight.colorAndIntensity.y;
		_color[2] = pointLight.colorAndIntensity.z;
		_intensity = pointLight.colorAndIntensity.w;

		_sourceRadius = pointLight.sourceRadius;

		_isVisible = pointLight.isVisible;
		_castShadows = pointLight.castShadows;

		_pointLight = &pointLight;
	}

	void PointLightWindow::draw_window(VulkanEngine* engine)
	{
		RenderScene& scene = engine->_renderScene;
	
		ImGui::Begin("Point light");

		if (ImGui::CollapsingHeader("Transform"))
		{		
			bool check = ImGui::DragFloat3("Translation", _position, 0.1f, -100000.0f, 100000.0f);
			scene._bNeedsBakePointShadows.push_back(check);
			scene._bNeedsReloadingPointLights.push_back(check);
		}
		if (ImGui::CollapsingHeader("Light"))
		{
			bool check = ImGui::SliderFloat("Source radius", &_sourceRadius, 0.0f, 1500.0f);
			scene._bNeedsReloadingPointLights.push_back(check);
			check = ImGui::SliderFloat("Attenuation radius", &_attenuationRadius, 0.0f, 1500.0f);
			scene._bNeedsReloadingPointLights.push_back(check);
			check = ImGui::Checkbox("Cast shadows", &_castShadows);
			scene._bNeedsReloadingPointLights.push_back(check);
			check = ImGui::ColorEdit3("Light color", _color);
			scene._bNeedsReloadingPointLights.push_back(check);
			check = ImGui::SliderFloat("Intensity (lm)", &_intensity, 0.0f, 100000.0f);
			scene._bNeedsReloadingPointLights.push_back(check);
		}
		if (ImGui::CollapsingHeader("Rendering"))
		{
			bool check = ImGui::Checkbox("Visible", &_isVisible);
			scene._bNeedsReloadingPointLights.push_back(check);
		}

		ImGui::End();
	}

	void PointLightWindow::set_point_light_data()
	{
		_pointLight->positionAndAttRadius = { _position[0], _position[1], _position[2], _attenuationRadius };
		_pointLight->colorAndIntensity = { _color[0], _color[1], _color[2], _intensity };
		_pointLight->sourceRadius = _sourceRadius;
		_pointLight->isVisible = _isVisible;
		_pointLight->castShadows = _castShadows;
	}

	DirLightWindow::DirLightWindow(actors::DirectionLight& dirLight)
	{
		_color[0] = dirLight.colorAndIntensity.x;
		_color[1] = dirLight.colorAndIntensity.y;
		_color[2] = dirLight.colorAndIntensity.z;

		_intensity = dirLight.colorAndIntensity.w;

		_isVisible = dirLight.isVisible;
		_castShadows = dirLight.castShadows;

		_dirLight = &dirLight;
	}

	void DirLightWindow::draw_window(VulkanEngine* engine)
	{
		RenderScene& scene = engine->_renderScene;
	
		ImGui::Begin("Directional light");

		if (ImGui::CollapsingHeader("Light"))
		{
			bool check = ImGui::Checkbox("Cast shadows", &_castShadows);
			scene._bNeedsRealoadingDirLights.push_back(check);
			check = ImGui::ColorEdit3("Light color", _color);
			scene._bNeedsRealoadingDirLights.push_back(check);
			check = ImGui::SliderFloat("Intensity (cd)", &_intensity, 0.0f, 1500.0f);
			scene._bNeedsRealoadingDirLights.push_back(check);
		}

		if (ImGui::CollapsingHeader("Rendering"))
		{
			bool check = ImGui::Checkbox("Visible", &_isVisible);
			scene._bNeedsRealoadingDirLights.push_back(check);
		}

		ImGui::End();
	}

	void DirLightWindow::set_dir_light_data()
	{
		_dirLight->colorAndIntensity = { _color[0], _color[1], _color[2], _intensity };
		_dirLight->isVisible = _isVisible;
		_dirLight->castShadows = _castShadows;
	}

	SpotLightWindow::SpotLightWindow(actors::SpotLight& spotLight)
	{
		_color[0] = spotLight.colorAndIntensity.x;
		_color[1] = spotLight.colorAndIntensity.y;
		_color[2] = spotLight.colorAndIntensity.z;
		_intensity = spotLight.colorAndIntensity.w;

		_positioin[0] = spotLight.positionAndDistance.x;
		_positioin[1] = spotLight.positionAndDistance.y;
		_positioin[2] = spotLight.positionAndDistance.z;
		_distance = spotLight.positionAndDistance.w;

		_rotation[0] = glm::degrees(spotLight.rotationAndInnerConeRadius.x);
		_rotation[1] = glm::degrees(spotLight.rotationAndInnerConeRadius.y);
		_rotation[2] = glm::degrees(spotLight.rotationAndInnerConeRadius.z);

		_innerConeRadius = glm::degrees(glm::acos(spotLight.rotationAndInnerConeRadius.w));
		_outerConeRadius = glm::degrees(glm::acos(spotLight.outerConeRadius));

		_isVisible = spotLight.isVisible;
		_castShadows = spotLight.castShadows;

		_spotLight = &spotLight;
	}

	void SpotLightWindow::draw_window(VulkanEngine* engine)
	{
		RenderScene& scene = engine->_renderScene;
	
		ImGui::Begin("Spot light");

		if (ImGui::CollapsingHeader("Transform"))
		{
			bool check = ImGui::DragFloat3("Translation", _positioin, 0.1f, -100000.0f, 100000.0f);
			//ImGui::DragFloat3()
			scene._bNeedsBakeSpotShadows.push_back(check);
			scene._bNeedsReloadingSpotLights.push_back(check);
			check = ImGui::DragFloat3("Rotation", _rotation, 0.25f, -720.0f, 720.0f);
			scene._bNeedsBakeSpotShadows.push_back(check);
			scene._bNeedsReloadingSpotLights.push_back(check);
		}
		if (ImGui::CollapsingHeader("Light"))
		{
			bool check = ImGui::SliderFloat("Inner cone angle", &_innerConeRadius, 0.0f, 80.0f);
			scene._bNeedsReloadingSpotLights.push_back(check);
			check = ImGui::SliderFloat("Outer cone angle", &_outerConeRadius, 0.0f, 80.0f);
			scene._bNeedsReloadingSpotLights.push_back(check);
			check = ImGui::SliderFloat("Disance", &_distance, 10.0f, 1000.0f);
			scene._bNeedsReloadingSpotLights.push_back(check);
			check = ImGui::Checkbox("Cast shadows", &_castShadows);
			scene._bNeedsReloadingSpotLights.push_back(check);
			check = ImGui::ColorEdit3("Light color", _color);
			scene._bNeedsReloadingSpotLights.push_back(check);
			check = ImGui::SliderFloat("Intensity (lm)", &_intensity, 0.0f, 100000.0f);
			scene._bNeedsReloadingSpotLights.push_back(check);
		}
		if (ImGui::CollapsingHeader("Rendering"))
		{
			bool check = ImGui::Checkbox("Visible", &_isVisible);
			scene._bNeedsReloadingSpotLights.push_back(check);
		}

		ImGui::End();
	}

	void SpotLightWindow::set_spot_light_data()
	{
		_spotLight->colorAndIntensity = { _color[0], _color[1], _color[2], _intensity };
		_rotation[0] = glm::radians(_rotation[0]);
		_rotation[1] = glm::radians(_rotation[1]);
		_rotation[2] = glm::radians(_rotation[2]);
		_spotLight->rotationAndInnerConeRadius = { _rotation[0], _rotation[1], _rotation[2], glm::cos(glm::radians(_innerConeRadius)) };
		_spotLight->positionAndDistance = {_positioin[0], _positioin[1], _positioin[2], _distance };
		_spotLight->outerConeRadius = glm::cos(glm::radians(_outerConeRadius));
		_spotLight->isVisible = _isVisible;
		_spotLight->castShadows = _castShadows;
	}

	SettingsWindow::SettingsWindow(Settings& settings)
	{
		_isTaaEnabled = settings.isTaaEnabled;
		_taaAlpha = settings.taaAlpha;
		_settings = &settings;
	}

	void SettingsWindow::draw_window()
	{
		ImGui::Begin("Settings");	
		ImGui::Checkbox("TAA", &_isTaaEnabled);
		if (_isTaaEnabled)
			ImGui::SliderFloat("TAA alpha", &_taaAlpha, 1.0f, 80.0f);
		ImGui::End();	
	}

	void SettingsWindow::set_settings_data()
	{
		_settings->isTaaEnabled = _isTaaEnabled;
		_settings->taaAlpha = _taaAlpha;
	}

}

