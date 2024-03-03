#include "viewport_window.h"
#include "application_core/editor_events.h"
#include "engine_core/basic_events.h"
#include "engine_core/basic_components.h"
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>

using namespace ad_astris;
using namespace editor;
using namespace impl;

ViewportWindow::ViewportWindow(UIWindowInitContext& initContext, std::function<uint64_t()> callback) : UIWindowInternal(initContext), _textureCallback(callback)
{
	subscribe_to_events();
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

	if (ImGui::IsKeyPressed(ImGuiKey_W) && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
		_gizmoType = ImGuizmo::OPERATION::TRANSLATE;
	else if (ImGui::IsKeyPressed(ImGuiKey_E))
		_gizmoType = ImGuizmo::OPERATION::ROTATE;
	else if (ImGui::IsKeyPressed(ImGuiKey_R))
		_gizmoType = ImGuizmo::OPERATION::SCALE;
	if (ImGui::IsKeyPressed(ImGuiKey_Tab))
		_gizmoType = -1;

	if (_selectedEntity && _gizmoType != -1)
		draw_gizmo();
	
	ImGui::End();
}

void ViewportWindow::draw_gizmo()
{
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist();
	float windowWidth = ImGui::GetWindowWidth();
	float windowHeight = ImGui::GetWindowHeight();
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
	
	auto cameraComponent = _activeCamera.get_component<ecore::CameraComponent>();
	auto& projection = cameraComponent->projection;
	auto& view = cameraComponent->view;

	auto entityTransformComponent = _selectedEntity.get_component<ecore::TransformComponent>();
	auto& world = entityTransformComponent->world;
	
	ImGuizmo::Manipulate(&view._11, &projection._11, (ImGuizmo::OPERATION)_gizmoType, ImGuizmo::MODE::WORLD, &world._11);

	if (ImGuizmo::IsUsing())
	{
		XMMATRIX mat = XMLoadFloat4x4(&world);
		XMVECTOR translation, rotation, scale;
		XMMatrixDecompose(&scale, &rotation, &translation, mat);
		//entityTransformComponent->rotationEuler = math::to_euler_angles(rotation);
		//entityTransformComponent->rotationEuler.x = XMConvertToDegrees(entityTransformComponent->rotationEuler.x);
		//entityTransformComponent->rotationEuler.y = XMConvertToDegrees(entityTransformComponent->rotationEuler.y);
		//entityTransformComponent->rotationEuler.z = XMConvertToDegrees(entityTransformComponent->rotationEuler.z);
		XMStoreFloat3(&entityTransformComponent->location, translation);
		XMStoreFloat4(&entityTransformComponent->rotation, rotation);
		XMStoreFloat3(&entityTransformComponent->scale, scale);
		entityTransformComponent->rotationEuler.x = XMConvertToDegrees(std::atan2f(world(1, 2), world(2, 2))); 
		entityTransformComponent->rotationEuler.y = XMConvertToDegrees(std::atan2f(-world(0, 2), sqrtf(world(1, 2) * world(1, 2) + world(2, 2) * world(2, 2)))); 
		entityTransformComponent->rotationEuler.z = XMConvertToDegrees(std::atan2f(world(0, 1), world(0, 0)));
	}
}

void ViewportWindow::subscribe_to_events()
{
	events::EventDelegate<ecore::EntityCreatedEvent> _activeCameraDelegate = [this](ecore::EntityCreatedEvent& event)
	{
		ecs::Entity entity = event.get_entity();
		if (event.get_entity_manager()->does_entity_have_component<ecore::CameraComponent>(entity))
		{
			auto camera = event.get_entity_manager()->get_component<ecore::CameraComponent>(entity);
			if (camera->isActive)
				_activeCamera = entity;
		}
	};
	_eventManager->subscribe(_activeCameraDelegate);
}
