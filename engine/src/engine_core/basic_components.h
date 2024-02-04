#pragma once

#include "ecs/ecs.h"
#include "core/math_base.h"
#include "core/reflection.h"
#include "core/attributes.h"

namespace ad_astris::ecore
{
	struct TransformComponent
	{
		COMPONENT_REFLECTOR_START(TransformComponent)
		FIELD(XMFLOAT3, location, (0.0f, 0.0f, 0.0f), Serializable(), EditAnywhere())
		FIELD(XMFLOAT4, rotation, (0.0f, 0.0f, 0.0f, 1.0f), Serializable(), EditAnywhere())
		FIELD(XMFLOAT3, rotationEuler, (0.0f, 0.0f, 0.0f), Serializable(), EditAnywhere())
		FIELD(XMFLOAT3, scale, (1.0f, 1.0f, 1.0f))
		REFLECTOR_END()
		
		XMFLOAT4X4 world = math::IDENTITY_MATRIX;
	};
	
	struct ModelComponent
	{
		COMPONENT_REFLECTOR_START(ModelComponent)
		FIELD(UUID, modelUUID, (), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};

	struct TextureComponent
	{
		COMPONENT_REFLECTOR_START(TextureComponent)
		FIELD(UUID, textureUUID, (), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};

	struct OpaquePBRMaterialComponent
	{
		COMPONENT_REFLECTOR_START(OpaquePBRMaterialComponent)
		FIELD(UUID, materialUUID, (), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};

	struct TransparentMaterialComponent
	{
		COMPONENT_REFLECTOR_START(TransparentMaterialComponent)
		FIELD(UUID, materialUUID, (), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};

	struct LuminanceIntensityComponent
	{
		COMPONENT_REFLECTOR_START(LuminanceIntensityComponent)
		FIELD(float, intensity, (), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};

	struct CandelaIntensityComponent
	{
		COMPONENT_REFLECTOR_START(CandelaIntensityComponent)
		FIELD(float, intensity, (), Serializable(), EditAnywhere());
		REFLECTOR_END()
	};

	struct IntensityComponent
	{
		COMPONENT_REFLECTOR_START(IntensityComponent)
		FIELD(float, intensity, (), Serializable(), EditAnywhere())		// For point and spot lights in candela, for directional lights in lux
		REFLECTOR_END()
	};

	struct ColorComponent
	{
		COMPONENT_REFLECTOR_START(ColorComponent)
		FIELD(XMFLOAT4, color, (), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};

	struct AttenuationRadiusComponent
	{
		COMPONENT_REFLECTOR_START(AttenuationRadiusComponent)
		FIELD(float, attenuationRadius, (), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};

	struct InnerConeAngleComponent
	{
		COMPONENT_REFLECTOR_START(InnerConeAngleComponent)
		FIELD(float, innerConeAngle, (), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};
	
	struct OuterConeAngleComponent
	{
		COMPONENT_REFLECTOR_START(OuterConeAngleComponent)
		FIELD(float, outerConeAngle, (), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};

	struct LightTemperatureComponent
	{
		COMPONENT_REFLECTOR_START(LightTemperatureComponent)
		FIELD(bool, isTemperatureUsed, (), Serializable(), EditAnywhere())
		FIELD(float, temperature, (), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};

	struct CastShadowComponent
	{
		COMPONENT_REFLECTOR_START(CastShadowComponent)
		FIELD(bool, castShadows, (), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};

	struct AffectWorldComponent
	{
		COMPONENT_REFLECTOR_START(AffectWorldComponent)
		FIELD(bool, isWorldAffected, (), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};

	struct VisibleComponent
	{
		COMPONENT_REFLECTOR_START(VisibleComponent)
		FIELD(bool, isVisible, (), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};

	struct ExtentComponent
	{
		COMPONENT_REFLECTOR_START(ExtentComponent)
		FIELD(uint32_t, width, (), Serializable(), EditAnywhere())
		FIELD(uint32_t, height, (), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};

	struct CameraComponent
	{
		COMPONENT_REFLECTOR_START(CameraComponent)
		FIELD(float, zNear, (0.1f), Serializable(), EditAnywhere())
		FIELD(float, zFar, (10000.0f), Serializable(), EditAnywhere())
		FIELD(float, fov, (50.0f), Serializable(), EditAnywhere())
		FIELD(bool, isActive, (false), Serializable(), EditAnywhere())
		FIELD(float, movementSpeed, (10.0f), Serializable(), EditAnywhere())
		FIELD(float, mouseSensitivity, (0.1f), Serializable(), EditAnywhere())
		REFLECTOR_END()

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
	
	struct StaticObjectTag { TAG_REFLECTOR(StaticObjectTag) };
	struct MovableObjectTag { TAG_REFLECTOR(MovableObjectTag) };
	struct PointLightTag { TAG_REFLECTOR(PointLightTag) };
	struct DirectionalLightTag { TAG_REFLECTOR(DirectionalLightTag) };
	struct SpotLightTag { TAG_REFLECTOR(SpotLightTag) };

	inline void register_basic_components(ecs::EntityManager* entityManager, uicore::ECSUiManager* ecsUIManager)
	{
		ecs::register_component<TransformComponent>(entityManager, ecsUIManager);
		ecs::register_component<ModelComponent>(entityManager, ecsUIManager);
		ecs::register_component<TextureComponent>(entityManager, ecsUIManager);
		ecs::register_component<OpaquePBRMaterialComponent>(entityManager, ecsUIManager);
		ecs::register_component<TransparentMaterialComponent>(entityManager, ecsUIManager);
		ecs::register_component<LuminanceIntensityComponent>(entityManager, ecsUIManager);
		ecs::register_component<CandelaIntensityComponent>(entityManager, ecsUIManager);
		ecs::register_component<ColorComponent>(entityManager, ecsUIManager);
		ecs::register_component<AttenuationRadiusComponent>(entityManager, ecsUIManager);
		ecs::register_component<InnerConeAngleComponent>(entityManager, ecsUIManager);
		ecs::register_component<OuterConeAngleComponent>(entityManager, ecsUIManager);
		ecs::register_component<LightTemperatureComponent>(entityManager, ecsUIManager);
		ecs::register_component<CastShadowComponent>(entityManager, ecsUIManager);
		ecs::register_component<AffectWorldComponent>(entityManager, ecsUIManager);
		ecs::register_component<VisibleComponent>(entityManager, ecsUIManager);
		ecs::register_component<ExtentComponent>(entityManager, ecsUIManager);
		ecs::register_component<CameraComponent>(entityManager, ecsUIManager);
		ecs::register_component<IntensityComponent>(entityManager, ecsUIManager);
		entityManager->register_tag<StaticObjectTag>();
		entityManager->register_tag<MovableObjectTag>();
		entityManager->register_tag<PointLightTag>();
		entityManager->register_tag<DirectionalLightTag>();
		entityManager->register_tag<SpotLightTag>();
	}
}
