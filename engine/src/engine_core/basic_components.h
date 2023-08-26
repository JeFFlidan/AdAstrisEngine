#pragma once

#include "ecs.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace ad_astris::ecore
{
	struct TransformComponent
	{
		glm::vec3 location{ 0.0f };
		glm::vec3 rotation{ 0.0f };
		glm::vec3 scale{ 1.0f };
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

	struct ColorComponent
	{
		glm::vec4 color;
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
		bool isShadowCast;
	};

	struct AffectWorldComponent
	{
		bool isWorldAffected;
	};

	struct VisibleComponent
	{
		bool isVisible;
	};
	
	struct StaticObjectTag { };
	struct MovableObjectTag { };
	struct PointLightTag { };
	struct DirectionalLightTag { };
	struct SpotLightTag { };
}

REGISTER_TAG(MovableObjectTag, ad_astris::ecore::MovableObjectTag)
REGISTER_TAG(PointLightTag, ad_astris::ecore::PointLightTag)
REGISTER_TAG(DirectionalLightTag, ad_astris::ecore::DirectionalLightTag)
REGISTER_TAG(StaticObjectTag, ad_astris::ecore::StaticObjectTag)
REGISTER_TAG(SpotLightTag, ad_astris::ecore::SpotLightTag)

REGISTER_COMPONENT(TransformComponent, ad_astris::ecore::TransformComponent, location, rotation, scale)
REGISTER_COMPONENT(ModelComponent, ad_astris::ecore::ModelComponent, modelUUID)
REGISTER_COMPONENT(TextureComponent, ad_astris::ecore::TextureComponent, textureUUID)
REGISTER_COMPONENT(LuminanceIntensityComponent, ad_astris::ecore::LuminanceIntensityComponent, intensity)
REGISTER_COMPONENT(CandelaIntensityComponent, ad_astris::ecore::CandelaIntensityComponent, intensity)
REGISTER_COMPONENT(ColorComponent, ad_astris::ecore::ColorComponent, color)
REGISTER_COMPONENT(AttenuationRadiusComponent, ad_astris::ecore::AttenuationRadiusComponent, attenuationRadius)
REGISTER_COMPONENT(InnerConeAngleComponent, ad_astris::ecore::InnerConeAngleComponent, angle)
REGISTER_COMPONENT(OuterConeAngleComponent, ad_astris::ecore::OuterConeAngleComponent, angle)
REGISTER_COMPONENT(LightTemperatureComponent, ad_astris::ecore::LightTemperatureComponent, isTemperatureUsed, temperature)
REGISTER_COMPONENT(CastShadowComponent, ad_astris::ecore::CastShadowComponent, isShadowCast)
REGISTER_COMPONENT(AffectWorldComponent, ad_astris::ecore::AffectWorldComponent, isWorldAffected)
REGISTER_COMPONENT(VisibleComponent, ad_astris::ecore::VisibleComponent, isVisible)
