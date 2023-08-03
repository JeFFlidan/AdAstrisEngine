#pragma once

#include "ecs.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace ad_astris
{
	struct TransformComponent
	{
		glm::vec3 location{ 0.0f };
		glm::vec3 rotation{ 0.0f };
		glm::vec3 scale{ 1.0f };
	};
	ECS_COMPONENT(TransformComponent, glm::vec3, glm::vec3, glm::vec3)
	
	struct ModelComponent
	{
		UUID uuid;
	};
	ECS_COMPONENT(ModelComponent, UUID)

	struct LuminanceIntensityComponent
	{
		float intensity;
	};
	ECS_COMPONENT(LuminanceIntensityComponent, float)

	struct CandelaIntensityComponent
	{
		float intensity;
	};
	ECS_COMPONENT(CandelaIntensityComponent, float)

	struct ColorComponent
	{
		glm::vec3 color;
	};
	ECS_COMPONENT(ColorComponent, glm::vec3)

	struct AttenuationRadiusComponent
	{
		float attenuationRadius;
	};
	ECS_COMPONENT(AttenuationRadiusComponent, float)

	struct InnerConeAngleComponent
	{
		float angle;
	};
	ECS_COMPONENT(InnerConeAngleComponent, float)
	
	struct OuterConeAngleComponent
	{
		float angle;
	};
	ECS_COMPONENT(OuterConeAngleComponent, float)

	struct UseLightTemperatureComponent
	{
		bool isTemperatureUsed;
	};
	ECS_COMPONENT(UseLightTemperatureComponent, bool)

	struct LightTemperatureComponent
	{
		float temperature;
	};
	ECS_COMPONENT(LightTemperatureComponent, float)

	struct CastShadowComponent
	{
		bool isShadowCast;
	};
	ECS_COMPONENT(CastShadowComponent, bool)

	struct AffectWorldComponent
	{
		bool isWorldAffected;
	};
	ECS_COMPONENT(AffectWorldComponent, bool)

	struct VisibleComponent
	{
		bool isVisible;
	};
	ECS_COMPONENT(VisibleComponent, bool)

	struct StaticObjectTag { };
	ECS_TAG(StaticObjectTag)

	struct MovableObjectTag { };
	ECS_TAG(MovableObjectTag)

	struct PointLightTag { };
	ECS_TAG(PointLightTag)

	struct DirectionalLightTag { };
	ECS_TAG(DirectionalLightTag)

	struct SpotLightTag { };
	ECS_TAG(SpotLightTag)
}
