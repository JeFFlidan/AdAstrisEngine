#pragma once

#include "ecs/ecs.h"
#include "core/math_base.h"

namespace ad_astris::ecore
{
	struct TransformComponent
	{
		XMFLOAT3 location{ 0.0f, 0.0f, 0.0f };
		XMFLOAT4 rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		XMFLOAT3 scale{ 1.0f, 1.0f, 1.0f };
		XMFLOAT4X4 world = math::IDENTITY_MATRIX;
	};
	
	struct ModelComponent
	{
		UUID modelUUID;
	};

	struct TextureComponent
	{
		UUID textureUUID;
	};

	struct OpaquePBRMaterialComponent
	{
		UUID materialUUID;
	};

	struct TransparentMaterialComponent
	{
		UUID materialUUID;
	};

	struct LuminanceIntensityComponent
	{
		float intensity;
	};

	struct CandelaIntensityComponent
	{
		float intensity;
	};

	struct IntensityComponent
	{
		float intensity;		// For point and spot lights in candela, for directional lights in lux 
	};

	struct ColorComponent
	{
		XMFLOAT4 color;
	};

	struct AttenuationRadiusComponent
	{
		float attenuationRadius;
	};

	struct InnerConeAngleComponent
	{
		float angle;
	};
	
	struct OuterConeAngleComponent
	{
		float angle;
	};

	struct LightTemperatureComponent
	{
		bool isTemperatureUsed;
		float temperature;
	};

	struct CastShadowComponent
	{
		bool castShadows;
	};

	struct AffectWorldComponent
	{
		bool isWorldAffected;
	};

	struct VisibleComponent
	{
		bool isVisible;
	};

	struct ExtentComponent
	{
		uint32_t width;
		uint32_t height;
	};

	struct CameraComponent
	{
		float zNear = 0.1f;
		float zFar = 10000.0f;
		float fov = 70.0f;
		bool isActive{ false };
		float movementSpeed{ 10.0f };
		float mouseSensitivity{ 0.1f };

		XMFLOAT3 eye = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 at = XMFLOAT3(0.0f, 0.0f, 1.0f);
		XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
		XMFLOAT4X4 viewProjection;
		XMFLOAT4X4 inverseView;
		XMFLOAT4X4 inverseProjection;
		XMFLOAT4X4 inverseViewProjection;
	};
	
	struct StaticObjectTag { };
	struct MovableObjectTag { };
	struct PointLightTag { };
	struct DirectionalLightTag { };
	struct SpotLightTag { };

	inline void register_basic_components(ecs::EntityManager* entityManager)
	{
		entityManager->register_component<TransformComponent>(true);
		entityManager->register_component<ModelComponent>(true);
		entityManager->register_component<TextureComponent>(true);
		entityManager->register_component<OpaquePBRMaterialComponent>(true);
		entityManager->register_component<TransparentMaterialComponent>(true);
		entityManager->register_component<LuminanceIntensityComponent>(true);
		entityManager->register_component<CandelaIntensityComponent>(true);
		entityManager->register_component<ColorComponent>(true);
		entityManager->register_component<AttenuationRadiusComponent>(true);
		entityManager->register_component<InnerConeAngleComponent>(true);
		entityManager->register_component<OuterConeAngleComponent>(true);
		entityManager->register_component<LightTemperatureComponent>(true);
		entityManager->register_component<CastShadowComponent>(true);
		entityManager->register_component<AffectWorldComponent>(true);
		entityManager->register_component<VisibleComponent>(true);
		entityManager->register_component<ExtentComponent>(true);
		entityManager->register_component<CameraComponent>(true);
		entityManager->register_component<IntensityComponent>(true);
		entityManager->register_tag<StaticObjectTag>();
		entityManager->register_tag<MovableObjectTag>();
		entityManager->register_tag<PointLightTag>();
		entityManager->register_tag<DirectionalLightTag>();
		entityManager->register_tag<SpotLightTag>();
	}
}

REFLECT_COMPONENT(ad_astris::ecore::TransformComponent, location, rotation, scale)
REFLECT_COMPONENT(ad_astris::ecore::ModelComponent, modelUUID)
REFLECT_COMPONENT(ad_astris::ecore::TextureComponent, textureUUID)
REFLECT_COMPONENT(ad_astris::ecore::OpaquePBRMaterialComponent, materialUUID)
REFLECT_COMPONENT(ad_astris::ecore::TransparentMaterialComponent, materialUUID)
REFLECT_COMPONENT(ad_astris::ecore::LuminanceIntensityComponent, intensity)
REFLECT_COMPONENT(ad_astris::ecore::CandelaIntensityComponent, intensity)
REFLECT_COMPONENT(ad_astris::ecore::ColorComponent, color)
REFLECT_COMPONENT(ad_astris::ecore::AttenuationRadiusComponent, attenuationRadius)
REFLECT_COMPONENT(ad_astris::ecore::InnerConeAngleComponent, angle)
REFLECT_COMPONENT(ad_astris::ecore::OuterConeAngleComponent, angle)
REFLECT_COMPONENT(ad_astris::ecore::LightTemperatureComponent, isTemperatureUsed, temperature)
REFLECT_COMPONENT(ad_astris::ecore::CastShadowComponent, castShadows)
REFLECT_COMPONENT(ad_astris::ecore::AffectWorldComponent, isWorldAffected)
REFLECT_COMPONENT(ad_astris::ecore::VisibleComponent, isVisible)
REFLECT_COMPONENT(ad_astris::ecore::ExtentComponent, width, height)
REFLECT_COMPONENT(ad_astris::ecore::CameraComponent, zNear, zFar, fov, isActive, movementSpeed, mouseSensitivity)
REFLECT_COMPONENT(ad_astris::ecore::IntensityComponent, intensity)
