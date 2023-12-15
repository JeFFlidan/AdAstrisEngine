﻿#include "entity_submanager.h"
#include "engine_core/engine_object_events.h"
#include "core/timer.h"

using namespace ad_astris::renderer::impl;

constexpr XMFLOAT3 POINT_LIGHT_CENTER_VECTORS[6] = {
	XMFLOAT3(1.0f, 0.0f, 0.0f),
	XMFLOAT3(-1.0f, 0.0f, 0.0f),
	XMFLOAT3(0.0f, 1.0f, 0.0f),
	XMFLOAT3(0.0f, -1.0f, 0.0f),
	XMFLOAT3(0.0f, 0.0f, 1.0f),
	XMFLOAT3(0.0f, 0.0f, -1.0f)
};

constexpr XMFLOAT3 POINT_LIGHT_UP_VECTORS[6] = {
	XMFLOAT3(0.0f, -1.0f, 0.0f),
	XMFLOAT3(0.0f, -1.0f, 0.0f),
	XMFLOAT3(0.0f, 0.0f, 1.0f),
	XMFLOAT3(0.0f, 0.0f, -1.0f),
	XMFLOAT3(0.0f, -1.0f, 0.0f),
	XMFLOAT3(0.0f, -1.0f, 0.0f)
};

constexpr uint32_t DISPATCH_GROUP_SIZE = 64u;
constexpr uint32_t RENDERER_ENTITIES_INIT_NUMBER = 512;

EntitySubmanager::EntitySubmanager(SceneSubmanagerInitializationContext& initContext) : SceneSubmanagerBase(initContext)
{
	subscribe_to_events();
	LightSubmanagerUpdatedEvent event;
	_eventManager->trigger_event(event);
	_engineEntities.reserve(512);
	_rendererEntities.reserve(512);
}

void EntitySubmanager::update(rhi::CommandBuffer& cmdBuffer)
{
	if (!_areGPUBuffersAllocated)
		allocate_gpu_buffers();

	update_cpu_arrays();
	update_gpu_buffer(cmdBuffer);
}

void EntitySubmanager::cleanup_after_update()
{
	_rendererEntities.clear();
}

bool EntitySubmanager::need_update()
{
	return true;
}

void EntitySubmanager::subscribe_to_events()
{
	
}

void EntitySubmanager::allocate_gpu_buffers()
{
	uint64_t initSize = RENDERER_ENTITIES_INIT_NUMBER * sizeof(RendererEntity);
	uint64_t loadedEntitiesSize = _engineEntities.size() * sizeof(RendererEntity);
	
	if (loadedEntitiesSize > initSize)
	{
		_rendererResourceManager->allocate_storage_buffer(RENDERER_ENTITY_BUFFER_NAME, loadedEntitiesSize);
	}
	else
	{
		_rendererResourceManager->allocate_storage_buffer(RENDERER_ENTITY_BUFFER_NAME, initSize);
	}
	
	_areGPUBuffersAllocated = true;
}

void EntitySubmanager::update_cpu_arrays()
{
	ecs::EntityManager* entityManager = _world->get_entity_manager();
	_pointLightCount.store(0);
	_spotLightCount.store(0);
	_directionalLightCount.store(0);

	tasks::TaskGroup taskGroup;
	_taskComposer->dispatch(taskGroup, _engineEntities.size(), DISPATCH_GROUP_SIZE, [&](tasks::TaskExecutionInfo execInfo)
	{
		ecs::Entity entity = _engineEntities[execInfo.globalTaskIndex];
		setup_light(entity);
	});
	_taskComposer->wait(taskGroup);
}

void EntitySubmanager::update_gpu_buffer(rhi::CommandBuffer& cmd)
{
	if (!_rendererEntities.empty())
	{
		_rendererResourceManager->update_buffer(
		   &cmd,
		   RENDERER_ENTITY_BUFFER_NAME,
		   sizeof(RendererEntity),
		   _rendererEntities.data(),
		   _rendererEntities.size(),
		   _rendererEntities.size());
	}
}

void EntitySubmanager::setup_light(ecs::Entity entity)
{
	_rendererEntitiesMutex.lock();
	RendererEntity& rendererEntity = _rendererEntities.emplace_back();
	_rendererEntitiesMutex.unlock();

	ecs::EntityManager* entityManager = _world->get_entity_manager();
	auto color = entityManager->get_component_const<ecore::ColorComponent>(entity)->color;
	auto intensity = entityManager->get_component_const<ecore::IntensityComponent>(entity)->intensity;
	rendererEntity.set_color(XMFLOAT4(color.x * intensity, color.y * intensity, color.z * intensity, color.w));
	auto transformComponent = entityManager->get_component_const<ecore::TransformComponent>(entity);
	rendererEntity.location = transformComponent->location;

	auto isVisible = entityManager->get_component<ecore::VisibleComponent>(entity)->isVisible;
	auto isWorldAffected = entityManager->get_component<ecore::AffectWorldComponent>(entity)->isWorldAffected;
	if (!isVisible || !isWorldAffected)
		return;

	if (entityManager->does_entity_have_tag<ecore::DirectionalLightTag>(entity))
	{
		rendererEntity.set_type(DIRECTIONAL_LIGHT);
		XMMATRIX worldMatrix = XMLoadFloat4x4(&transformComponent->world);
		XMVECTOR direction = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), worldMatrix));
		XMFLOAT3 directionStorage;
		XMStoreFloat3(&directionStorage, direction);
		rendererEntity.set_direction(directionStorage);
		_directionalLightCount.fetch_add(1);
		return;
	}
	if (entityManager->does_entity_have_tag<ecore::PointLightTag>(entity))
	{
		rendererEntity.set_type(POINT_LIGHT);
		auto attenuationComponent = entityManager->get_component_const<ecore::AttenuationRadiusComponent>(entity);
		rendererEntity.set_attenuation_radius(attenuationComponent->attenuationRadius);
		_pointLightCount.fetch_add(1);
		return;
	}
	if (entityManager->does_entity_have_tag<ecore::SpotLightTag>(entity))
	{
		rendererEntity.set_type(SPOT_LIGHT);
		XMMATRIX worldMatrix = XMLoadFloat4x4(&transformComponent->world);
		XMVECTOR direction = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), worldMatrix));
		XMFLOAT3 directionStorage;
		XMStoreFloat3(&directionStorage, direction);
		rendererEntity.set_direction(directionStorage);

		auto attenuationComponent = entityManager->get_component_const<ecore::AttenuationRadiusComponent>(entity);
		rendererEntity.set_attenuation_radius(attenuationComponent->attenuationRadius);

		float outerConeAngle = entityManager->get_component_const<ecore::OuterConeAngleComponent>(entity)->outerConeAngle;
		float innerConeAngle = entityManager->get_component_const<ecore::InnerConeAngleComponent>(entity)->innerConeAngle;

		outerConeAngle = XMConvertToRadians(outerConeAngle);
		innerConeAngle = XMConvertToRadians(innerConeAngle);

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_lights_punctual#inner-and-outer-cone-angles
		float lightAngleScale = 1.0f / std::max(0.001f, std::cos(innerConeAngle) - std::cos(outerConeAngle));
		float lightAngleOffset = -std::cos(outerConeAngle) * lightAngleScale;

		rendererEntity.set_outer_cone_angle_cos(outerConeAngle);
		rendererEntity.set_angle_scale(lightAngleScale);
		rendererEntity.set_angle_offset(lightAngleOffset);
		_spotLightCount.fetch_add(1);
	}
}

void EntitySubmanager::setup_point_lights_matrices(tasks::TaskGroup& taskGroup)
{
	// for (auto& pointLightTemp : _recentlyCreated.pointLights)
	// {
	// 	PointLight& pointLight = _cpuCollections.pointLights[pointLightTemp.pointLightCollectionIndex];
 //        float aspect = (float)pointLightTemp.extent.width / (float)pointLightTemp.extent.height;
 //        float nearPlane = 0.1f;
 //        pointLight.farPlane = 1024.0f;
 //        XMFLOAT4X4 projMat = glm::perspective(glm::radians(90.0f), aspect, nearPlane, pointLight.farPlane);
 //        XMFLOAT3 pos = XMFLOAT3(pointLight.locationAndAttenuationRadius);
 //  
 //        for (uint32_t i = 0; i != 6; ++i)
 //        {
 //        	pointLight.lightSpaceMat[i] = projMat * glm::lookAt(pos, pos + POINT_LIGHT_CENTER_VECTORS[i], POINT_LIGHT_UP_VECTORS[i]);
 //        }
	// }
}

void EntitySubmanager::setup_directional_lights_matrices(tasks::TaskGroup& taskGroup)
{
	// for (auto& dirLightTemp : _recentlyCreated.directionalLights)
	// {
	// 	DirectionalLight& dirLight = _cpuCollections.directionalLights[dirLightTemp.directionalLightCollectionIndex];
	// 	float nearPlane = 0.01f;
	// 	float farPlane = 600.0f;
	// 	
	// 	XMFLOAT4X4 projMat = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, nearPlane, farPlane);
	// 	XMFLOAT3 pos = XMFLOAT3(dirLight.direction) * ((-farPlane) * 0.75f);
	// 	dirLight.lightSpaceMat = projMat * glm::lookAt(pos, XMFLOAT3(0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	// 	
	// }
}

void EntitySubmanager::setup_spot_lights_matrices(tasks::TaskGroup& taskGroup)
{
	// for (auto& spotLightTemp : _recentlyCreated.spotLights)
	// {
	// 	SpotLight& spotLight = _cpuCollections.spotLights[spotLightTemp.spotLightCollectionIndex];
	// 	float aspect = (float)spotLightTemp.extent.width / (float)spotLightTemp.extent.height;
	// 	float nearPlane = 0.1f;
	// 	float farPlane = 1024.0f;
	// 	
	// 	XMFLOAT4X4 projMat = glm::perspective(glm::radians(110.0f), aspect, nearPlane, farPlane);
	// 	
	// 	float rx = spotLight.rotationAndInnerConeRadius.x;
	// 	float ry = spotLight.rotationAndInnerConeRadius.y;
	// 	float rz = spotLight.rotationAndInnerConeRadius.z;
	// 	
	// 	XMFLOAT3 dir;
	// 	dir.x = glm::cos(ry) * glm::cos(rx);
	// 	dir.y = glm::sin(rx);
	// 	dir.z = glm::sin(ry) * glm::cos(rx);
	// 	
	// 	XMFLOAT3 pos = XMFLOAT3(spotLight.locationAndDistance);
	// 	
	// 	XMFLOAT4X4 viewMat = glm::lookAt(pos, pos + dir, XMFLOAT3(0.0f, 1.0f, 0.0f));
	// 	spotLight.lightSpaceMat = projMat * viewMat;
	// }
}