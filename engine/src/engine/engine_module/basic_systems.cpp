#include "basic_systems.h"

#include "engine_events.h"
#include "engine_core/basic_components.h"
#include "engine_core/basic_events.h"
#include "application_core/window_events.h"
#include "application_core/editor_events.h"

using namespace ad_astris;
using namespace engine;
using namespace impl;

constexpr uint32_t SMALL_GROUP_SIZE = 64;

void TransformUpdateSystem::subscribe_to_events(ecs::EngineManagers& managers)
{
	events::EventDelegate<ecore::LocalTransformUpdateEvent> delegate1 = [&](ecore::LocalTransformUpdateEvent& event)
	{
		ecs::Entity entity = event.get_entity();
		ecore::TransformComponent transform = managers.entityManager->get_entity_component<ecore::TransformComponent>(entity);
		if (event.need_location_update())
		{
			BasicSystemsUtils::translate(event.get_location(), transform.location);
		}
		if (event.need_rotation_update())
		{
			XMFLOAT3 eventRotation = event.get_rotation();
			XMFLOAT3 rotation = XMFLOAT3(XMConvertToRadians(eventRotation.x), XMConvertToRadians(eventRotation.y), XMConvertToRadians(eventRotation.z));
			BasicSystemsUtils::rotate(rotation, transform.rotation);
		}
		if (event.need_scale_update())
		{
			BasicSystemsUtils::scale(event.get_scale(), transform.scale);
		}
		
		managers.entityManager->set_entity_component(entity, transform);
	};
	managers.eventManager->subscribe(delegate1);
}

void TransformUpdateSystem::configure_query()
{
	_entityQuery.add_component_requirement<ecore::TransformComponent>(ecs::ComponentAccess::READ_WRITE);
}

void TransformUpdateSystem::configure_execution_order()
{
	
}

void TransformUpdateSystem::execute(ecs::EngineManagers& managers, tasks::TaskGroup& globalTaskGroup)
{
	tasks::TaskGroup& taskGroup = *managers.taskComposer->allocate_task_group();
	_entityQuery.for_each_chunk([&](ecs::ExecutionContext& execContext)
	{
		auto transformArrayView = execContext.get_mutable_components<ecore::TransformComponent>();
		managers.taskComposer->dispatch(taskGroup, execContext.get_entities_count(), SMALL_GROUP_SIZE, [=](tasks::TaskExecutionInfo tasksExecInfo) mutable
		{
			auto& transform = transformArrayView[tasksExecInfo.globalTaskIndex];
			XMVECTOR location = XMLoadFloat3(&transform.location);
			XMVECTOR rotation = XMLoadFloat4(&transform.rotation);
			XMVECTOR scale = XMLoadFloat3(&transform.scale);

			XMStoreFloat4x4(&transform.world,
				XMMatrixMultiply(XMMatrixMultiply(XMMatrixScalingFromVector(scale), XMMatrixRotationQuaternion(rotation)),
				XMMatrixTranslationFromVector(location)));
		});
	});
	managers.taskComposer->wait(taskGroup);
}

void CameraUpdateSystem::subscribe_to_events(ecs::EngineManagers& managers)
{
	events::EventDelegate<acore::DeltaTimeUpdateEvent> delegate1 = [&](acore::DeltaTimeUpdateEvent& event)
	{
		_deltaTime = event.get_delta_time();
	};
	managers.eventManager->subscribe(delegate1);

	events::EventDelegate<UpdateActiveCameraEvent> delegate2 = [&](UpdateActiveCameraEvent& event)
	{
		_activeCamera = event.get_entity();
	};
	managers.eventManager->subscribe(delegate2);

	events::EventDelegate<acore::InputEvent> delegate3 = [&](acore::InputEvent& event)
	{
		if (!event.is_mouse_button_pressed(acore::MouseButton::RIGHT))
			return;

		if (!event.is_viewport_hovered())
			return;
		
		auto transform = managers.entityManager->get_component<ecore::TransformComponent>(_activeCamera);
		auto camera = managers.entityManager->get_component_const<ecore::CameraComponent>(_activeCamera);

		XMFLOAT2 deltaPosition = event.get_delta_position();
		float xDelta = deltaPosition.x;
		float yDelta = deltaPosition.y;

		xDelta = camera->mouseSensitivity * xDelta * (1.0f / 60.0f);
		yDelta = camera->mouseSensitivity * yDelta * (1.0f / 60.0f);

		float clampedDeltaTime = std::min(_deltaTime, 0.1f);
		bool isShiftPressed = event.is_key_pressed(acore::Key::LEFT_SHIFT);
		float speed = camera->movementSpeed * (isShiftPressed ? 5.0f : 1.0f);

		XMVECTOR move = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		if (event.is_key_pressed(acore::Key::A))
			move = XMVectorAdd(move, XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
		if (event.is_key_pressed(acore::Key::D))
			move = XMVectorAdd(move, XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
		if (event.is_key_pressed(acore::Key::W))
			move = XMVectorAdd(move, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
		if (event.is_key_pressed(acore::Key::S))
			move = XMVectorAdd(move, XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));

		float velocity = speed * clampedDeltaTime;
		move = XMVectorMultiply(move, XMVectorSet(velocity, velocity, velocity, velocity));

		float moveLength = XMVectorGetX(XMVector3Length(move));

		if (abs(xDelta) + abs(yDelta) > 0 || moveLength > 0.0001f)
		{
			XMMATRIX cameraRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&transform->rotation));
			XMVECTOR rotatedMove = XMVector3TransformNormal(move, cameraRotation);
			XMFLOAT3 moveStorage;
			XMStoreFloat3(&moveStorage, rotatedMove);
			BasicSystemsUtils::translate(moveStorage, transform->location);
			BasicSystemsUtils::rotate(XMFLOAT3(yDelta, xDelta, 0.0f), transform->rotation);
		}
	};
	managers.eventManager->subscribe(delegate3);

	events::EventDelegate<acore::ViewportResizedEvent> delegate4 = [&](acore::ViewportResizedEvent& event)
	{
		_width = event.get_width();
		_height = event.get_height();
	};
	managers.eventManager->subscribe(delegate4);
}

void CameraUpdateSystem::configure_query()
{
	_entityQuery.add_component_requirement<ecore::CameraComponent>(ecs::ComponentAccess::READ_WRITE);
	_entityQuery.add_component_requirement<ecore::TransformComponent>(ecs::ComponentAccess::READ_ONLY);
}

void CameraUpdateSystem::configure_execution_order()
{
	_executionOrder.add_to_execute_after<TransformUpdateSystem>();
}

void CameraUpdateSystem::execute(ecs::EngineManagers& managers, tasks::TaskGroup& globalTaskGroup)
{
	tasks::TaskGroup& taskGroup = *managers.taskComposer->allocate_task_group();
	_entityQuery.for_each_chunk([&](ecs::ExecutionContext& execContext)
	{
		auto transformConstView = execContext.get_immutable_components<ecore::TransformComponent>();
		auto cameraView = execContext.get_mutable_components<ecore::CameraComponent>();
		managers.taskComposer->dispatch(taskGroup, execContext.get_entities_count(), SMALL_GROUP_SIZE, [=](tasks::TaskExecutionInfo execInfo)	mutable 
		{
			auto transformComponent = transformConstView[execInfo.globalTaskIndex];
			auto& cameraComponent = cameraView[execInfo.globalTaskIndex];
			XMMATRIX worldTransform = XMLoadFloat4x4(&transformComponent->world);
			XMVECTOR eye = XMVector3Transform(XMVectorSet(0, 0, 0, 1), worldTransform);
			XMVECTOR at = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), worldTransform));
			XMVECTOR up = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), worldTransform));
			XMStoreFloat3(&cameraComponent.eye, eye);
			XMStoreFloat3(&cameraComponent.at, at);
			XMStoreFloat3(&cameraComponent.up, up);
			
			XMMATRIX view = XMMatrixLookToLH(eye, at, up);
			XMMATRIX projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(cameraComponent.fov), _width / _height, cameraComponent.zNear, cameraComponent.zFar);
			XMMATRIX viewProjection = XMMatrixMultiply(view, projection);
			XMMATRIX invView = XMMatrixInverse(nullptr, view);
			XMMATRIX invProjection = XMMatrixInverse(nullptr, projection);
			XMMATRIX invViewProjection = XMMatrixInverse(nullptr, viewProjection);
			
			XMStoreFloat4x4(&cameraComponent.view, view);
			XMStoreFloat4x4(&cameraComponent.projection, projection);
			XMStoreFloat4x4(&cameraComponent.viewProjection, viewProjection);
			XMStoreFloat4x4(&cameraComponent.inverseView, invView);
			XMStoreFloat4x4(&cameraComponent.inverseProjection, invProjection);
			XMStoreFloat4x4(&cameraComponent.inverseViewProjection, invViewProjection);
		});
	});
	managers.taskComposer->wait(taskGroup);
}

void register_basic_systems(ecs::SystemManager* systemManager)
{
	systemManager->register_system<TransformUpdateSystem>();
	systemManager->register_system<CameraUpdateSystem>();
}

void BasicSystemsUtils::translate(const XMFLOAT3& input, XMFLOAT3& output)
{
	output.x += input.x;
	output.y += input.y;
	output.z += input.z;
}

void BasicSystemsUtils::rotate(const XMFLOAT3& input, XMFLOAT4& output)
{
	// Take from WickedEngine. Maybe, I will change this in the future
	XMVECTOR quat = XMLoadFloat4(&output);
	XMVECTOR x = XMQuaternionRotationRollPitchYaw(input.x, 0, 0);
	XMVECTOR y = XMQuaternionRotationRollPitchYaw(0, input.y, 0);
	XMVECTOR z = XMQuaternionRotationRollPitchYaw(0, 0, input.z);

	quat = XMQuaternionMultiply(x, quat);
	quat = XMQuaternionMultiply(quat, y);
	quat = XMQuaternionMultiply(z, quat);
	quat = XMQuaternionNormalize(quat);

	XMStoreFloat4(&output, quat);
}

void BasicSystemsUtils::scale(const XMFLOAT3& input, XMFLOAT3& output)
{
	output.x *= input.x;
	output.y *= input.y;
	output.z *= input.z;
}
