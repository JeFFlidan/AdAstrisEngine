#include "light_submanager.h"
#include "engine_core/engine_object_events.h"
#include "core/timer.h"

using namespace ad_astris::renderer::impl;

constexpr glm::vec3 POINT_LIGHT_CENTER_VECTORS[6] = {
	glm::vec3(1.0f, 0.0f, 0.0f),
	glm::vec3(-1.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, -1.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 1.0f),
	glm::vec3(0.0f, 0.0f, -1.0f)
};

constexpr glm::vec3 POINT_LIGHT_UP_VECTORS[6] = {
	glm::vec3(0.0f, -1.0f, 0.0f),
	glm::vec3(0.0f, -1.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 1.0f),
	glm::vec3(0.0f, 0.0f, -1.0f),
	glm::vec3(0.0f, -1.0f, 0.0f),
	glm::vec3(0.0f, -1.0f, 0.0f)
};

LightSubmanager::LightSubmanager(SceneSubmanagerInitializationContext& initContext) : SceneSubmanagerBase(initContext)
{
	subscribe_to_events();
	LightSubmanagerUpdatedEvent event;
	_eventManager->trigger_event(event);
}

void LightSubmanager::update(rhi::CommandBuffer& cmdBuffer)
{
	LOG_INFO("Before setup lights")
	setup_lights();
	LOG_INFO("After setup lights")
	update_gpu_buffer(cmdBuffer);
	LOG_INFO("After updating")
}

void LightSubmanager::cleanup_after_update()
{
	LOG_ERROR("RESET TEMP ARRAYS LIGHT SUBMANAGER")
	_recentlyCreated.pointLights.clear();
	_recentlyCreated.directionalLights.clear();
	_recentlyCreated.spotLights.clear();
	LOG_WARNING("FINISH TEMP ARRAY CLEANUPING")
}

bool LightSubmanager::need_update()
{
	return !_areGPUBuffersAllocated || !_recentlyCreated.pointLights.empty() || !_recentlyCreated.directionalLights.empty() || !_recentlyCreated.spotLights.empty();
}

void LightSubmanager::subscribe_to_events()
{
	LOG_INFO("LIGHT SUBMANAGER EVENTS REGISTRATION")
	events::EventDelegate<ecore::PointLightCreatedEvent> pointLightCreatedDelegate = [&](ecore::PointLightCreatedEvent& event)
	{
		PointLight pointLight;
		auto context = event.get_component_context();
		pointLight.color = context->colorComponent.color;
		pointLight.intensity = context->luminanceIntensityComponent.intensity;
		pointLight.temperature = context->lightTemperatureComponent.temperature;
		pointLight.isTemperatureUsed = context->lightTemperatureComponent.isTemperatureUsed;
		pointLight.castShadows = context->castShadowComponent.castShadows;
		pointLight.isWorldAffected = context->affectWorldComponent.isWorldAffected;
		pointLight.locationAndAttenuationRadius = glm::vec4(context->transformComponent.location, context->attenuationRadiusComponent.attenuationRadius);
		pointLight.isVisible = context->visibleComponent.isVisible;
		PointLightTemp pointLightTemp;
		pointLightTemp.extent = context->extentComponent;
		pointLightTemp.pointLightCollectionIndex = _cpuCollections.pointLights.add_object(pointLight, context->entity);
		_recentlyCreated.pointLights.push_back(pointLightTemp);
		LightSubmanagerUpdatedEvent submanagerUpdatedEvent;
		_eventManager->trigger_event(submanagerUpdatedEvent);
	};
	_eventManager->subscribe(pointLightCreatedDelegate);

	events::EventDelegate<ecore::DirectionalLightCreatedEvent> dirLightCreatedDelegate = [&](ecore::DirectionalLightCreatedEvent& event)
	{
		DirectionalLight dirLight;
		auto context = event.get_component_context();
		dirLight.color = context->colorComponent.color;
		dirLight.intensity = context->candelaIntensityComponent.intensity;
		dirLight.temperature = context->lightTemperatureComponent.temperature;
		dirLight.isTemperatureUsed = context->lightTemperatureComponent.isTemperatureUsed;
		dirLight.castShadows = context->castShadowComponent.castShadows;
		dirLight.isVisible = context->visibleComponent.isVisible;
		dirLight.isWorldAffected = context->affectWorldComponent.isWorldAffected;
		DirectionalLightTemp dirLightTemp;
		dirLightTemp.extent = context->extentComponent;
		dirLightTemp.directionalLightCollectionIndex = _cpuCollections.directionalLights.add_object(dirLight, context->entity);
		_recentlyCreated.directionalLights.push_back(dirLightTemp);
		LightSubmanagerUpdatedEvent submanagerUpdatedEvent;
		_eventManager->trigger_event(submanagerUpdatedEvent);
	};
	_eventManager->subscribe(dirLightCreatedDelegate);

	events::EventDelegate<ecore::SpotLightCreatedEvent> spotLigthCreatedDelegate = [&](ecore::SpotLightCreatedEvent& event)
	{
		SpotLight spotLight;
		auto context = event.get_component_context();
		spotLight.color = context->colorComponent.color;
		spotLight.intensity = context->luminanceIntensityComponent.intensity;
		spotLight.temperature = context->lightTemperatureComponent.temperature;
		spotLight.isTemperatureUsed = context->lightTemperatureComponent.isTemperatureUsed;
		spotLight.castShadows = context->castShadowComponent.castShadows;
		spotLight.isVisible = context->visibleComponent.isVisible;
		spotLight.isWorldAffected = context->affectWorldComponent.isWorldAffected;
		spotLight.locationAndDistance = glm::vec4(context->transformComponent.location, context->attenuationRadiusComponent.attenuationRadius);
		
		float innerConeRadius = glm::cos(glm::radians(context->innerConeAngleComponent.angle));
		float rx = glm::radians(context->transformComponent.rotation.x);
		float ry = glm::radians(context->transformComponent.rotation.y);
		float rz = glm::radians(context->transformComponent.rotation.z);
		spotLight.rotationAndInnerConeRadius = glm::vec4(rx, ry, rz, innerConeRadius);
		
		spotLight.outerConeRadius = glm::cos(glm::radians(context->outerConeAngleComponent.angle));
		SpotLightTemp spotLightTemp;
		spotLightTemp.extent = context->extentComponent;
		spotLightTemp.spotLightCollectionIndex = _cpuCollections.spotLights.add_object(spotLight, context->entity);
		_recentlyCreated.spotLights.push_back(spotLightTemp);
		LightSubmanagerUpdatedEvent submanagerUpdatedEvent;
		_eventManager->trigger_event(submanagerUpdatedEvent);
	};
	_eventManager->subscribe(spotLigthCreatedDelegate);
}

void LightSubmanager::setup_lights()
{
	tasks::TaskGroup taskGroup;
	Timer timer;

	setup_point_lights_matrices(taskGroup);
	setup_directional_lights_matrices(taskGroup);
	setup_spot_lights_matrices(taskGroup);

	_taskComposer->wait(taskGroup);
	LOG_INFO("LightSubmanager::setup_lights(): Time: {} ms", timer.elapsed_milliseconds())
}

void LightSubmanager::allocate_gpu_buffers()
{
	if (_recentlyCreated.pointLights.size() * sizeof(PointLight) > POINT_LIGHT_DEFAULT_COUNT * sizeof(PointLight))
		_rendererResourceManager->allocate_storage_buffer(POINT_LIGHT_BUFFER_NAME, _recentlyCreated.pointLights.size() * sizeof(PointLight));
	else
		_rendererResourceManager->allocate_storage_buffer(POINT_LIGHT_BUFFER_NAME, POINT_LIGHT_DEFAULT_COUNT * sizeof(PointLight));

	if (_recentlyCreated.spotLights.size() * sizeof(SpotLight) > SPOT_LIGHT_DEFAULT_COUNT * sizeof(SpotLight))
		_rendererResourceManager->allocate_storage_buffer(SPOT_LIGHT_BUFFER_NAME, _recentlyCreated.spotLights.size() * sizeof(SpotLight));
	else
		_rendererResourceManager->allocate_storage_buffer(SPOT_LIGHT_BUFFER_NAME, SPOT_LIGHT_DEFAULT_COUNT * sizeof(SpotLight));

	if (_recentlyCreated.directionalLights.size() > sizeof(DirectionalLight) > DIRECTIONAL_LIGHT_DEFAULT_COUNT * sizeof(DirectionalLight))
		_rendererResourceManager->allocate_storage_buffer(DIR_LIGHT_BUFFER_NAME, _recentlyCreated.directionalLights.size() * sizeof(DirectionalLight));
	else
		_rendererResourceManager->allocate_storage_buffer(DIR_LIGHT_BUFFER_NAME, DIRECTIONAL_LIGHT_DEFAULT_COUNT * sizeof(DirectionalLight));
	
	_areGPUBuffersAllocated = true;
}

void LightSubmanager::update_gpu_buffer(rhi::CommandBuffer& cmd)
{
	if (!_areGPUBuffersAllocated)
		allocate_gpu_buffers();
	
	if (!_recentlyCreated.pointLights.empty())
	{
		_rendererResourceManager->update_buffer(
		   &cmd,
		   POINT_LIGHT_BUFFER_NAME,
		   sizeof(PointLight),
		   _cpuCollections.pointLights.begin(),
		   _cpuCollections.pointLights.size(),
		   _recentlyCreated.pointLights.size());
	}
	if (!_recentlyCreated.spotLights.empty())
	{
		_rendererResourceManager->update_buffer(
			&cmd,
			SPOT_LIGHT_BUFFER_NAME,
			sizeof(SpotLight),
			_cpuCollections.spotLights.begin(),
			_cpuCollections.spotLights.size(),
			_recentlyCreated.spotLights.size());
	}
	if (!_recentlyCreated.directionalLights.empty())
	{
		_rendererResourceManager->update_buffer(
			&cmd,
			DIR_LIGHT_BUFFER_NAME,
			sizeof(DirectionalLight),
			_cpuCollections.directionalLights.begin(),
			_cpuCollections.directionalLights.size(),
			_recentlyCreated.directionalLights.size());
	}
}

void LightSubmanager::setup_point_lights_matrices(tasks::TaskGroup& taskGroup)
{
	for (auto& pointLightTemp : _recentlyCreated.pointLights)
	{
		PointLight& pointLight = _cpuCollections.pointLights[pointLightTemp.pointLightCollectionIndex];
        float aspect = (float)pointLightTemp.extent.width / (float)pointLightTemp.extent.height;
        float nearPlane = 0.1f;
        pointLight.farPlane = 1024.0f;
        glm::mat4 projMat = glm::perspective(glm::radians(90.0f), aspect, nearPlane, pointLight.farPlane);
        projMat[1][1] *= -1;
        glm::vec3 pos = glm::vec3(pointLight.locationAndAttenuationRadius);
 
        for (uint32_t i = 0; i != 6; ++i)
        {
        	pointLight.lightSpaceMat[i] = projMat * glm::lookAt(pos, pos + POINT_LIGHT_CENTER_VECTORS[i], POINT_LIGHT_UP_VECTORS[i]);
        }
	}
}

void LightSubmanager::setup_directional_lights_matrices(tasks::TaskGroup& taskGroup)
{
	for (auto& dirLightTemp : _recentlyCreated.directionalLights)
	{
		DirectionalLight& dirLight = _cpuCollections.directionalLights[dirLightTemp.directionalLightCollectionIndex];
		float nearPlane = 0.01f;
		float farPlane = 600.0f;
	
		glm::mat4 projMat = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, nearPlane, farPlane);
		projMat[1][1] *= -1;
		glm::vec3 pos = glm::vec3(dirLight.direction) * ((-farPlane) * 0.75f);
		dirLight.lightSpaceMat = projMat * glm::lookAt(pos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	
	}
}

void LightSubmanager::setup_spot_lights_matrices(tasks::TaskGroup& taskGroup)
{
	for (auto& spotLightTemp : _recentlyCreated.spotLights)
	{
		SpotLight& spotLight = _cpuCollections.spotLights[spotLightTemp.spotLightCollectionIndex];
		float aspect = (float)spotLightTemp.extent.width / (float)spotLightTemp.extent.height;
		float nearPlane = 0.1f;
		float farPlane = 1024.0f;
	
		glm::mat4 projMat = glm::perspective(glm::radians(110.0f), aspect, nearPlane, farPlane);
		projMat[1][1] *= -1;
		
		float rx = spotLight.rotationAndInnerConeRadius.x;
		float ry = spotLight.rotationAndInnerConeRadius.y;
		float rz = spotLight.rotationAndInnerConeRadius.z;
		
		glm::vec3 dir;
		dir.x = glm::cos(ry) * glm::cos(rx);
		dir.y = glm::sin(rx);
		dir.z = glm::sin(ry) * glm::cos(rx);
		
		glm::vec3 pos = glm::vec3(spotLight.locationAndDistance);
		
		glm::mat4 viewMat = glm::lookAt(pos, pos + dir, glm::vec3(0.0f, 1.0f, 0.0f));
		spotLight.lightSpaceMat = projMat * viewMat;
	}
}
