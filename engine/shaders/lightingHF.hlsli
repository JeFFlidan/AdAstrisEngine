#ifndef LIGHTING_HF
#define LIGHTING_HF

#include "common.hlsli"
#include "surfaceHF.hlsli"

inline float calculate_smooth_distance_attenuation(in float sqrDistance, in float invSqrAttRadius)
{
	float factor = sqrDistance * invSqrAttRadius;
	float smoothFactor = clamp(1.0 - pow(factor, 8.0f), 0.0f, 1.0f);
	return smoothFactor * smoothFactor;
}

inline float calculate_distance_attenuation(in float3 unnormalizedLightVector, in float invSqrAttRadius)
{
	float sqrDistance = dot(unnormalizedLightVector, unnormalizedLightVector);
	float attenuation = 1.0f / max(sqrDistance, 0.01 * 0.01);
	attenuation *= calculate_smooth_distance_attenuation(sqrDistance, invSqrAttRadius);
	return attenuation;
}

// inline float calculate_point_light_attenuation(in float3 unnormalizedLightVector, in float attenuationRadius)
// {
// 	float lightInvSqrAttenuationRadius = pow(1.0 / attenuationRadius, 2.0f);
// 	return calculate_distance_attenuation(unnormalizedLightVector, lightInvSqrAttenuationRadius);
// }

inline float calculate_point_light_attenuation(in float3 unnormalizedLightVector, in float attenuationRadius)
{
	float3 L = unnormalizedLightVector;
	float distance = length(L);
	float d = max(distance - attenuationRadius, 0.0);

	float denominator = d / attenuationRadius + 1;
	float attenuation = 1 / (denominator * denominator);

	float cutoff = 0.005f;		// Have to think how to calculate it adaptable
	attenuation = (attenuation - cutoff) / (1 - cutoff);
	return max(attenuation, 0.0);
}

inline float calculate_spot_light_attenuation(
	in float3 spotLightDir,
	in float3 unnormalizedLightVector,
	in float attenuationRadius,
	in float angleScale,
	in float angleOffset)
{
	float distanceAttenuation = calculate_point_light_attenuation(unnormalizedLightVector, attenuationRadius);
	float spotFactor = dot(spotLightDir, unnormalizedLightVector);
	float angularAttenuation = saturate(mad(spotFactor, angleScale, angleOffset));
	angularAttenuation *= angularAttenuation;
	return distanceAttenuation * angularAttenuation;
}


inline float3 calculate_point_light(in RendererEntity entity, in SurfaceDesc surface)
{
	float3 unnormalizedLightVector = entity.location - surface.location;
	float3 normalizedLightVector = normalize(unnormalizedLightVector);
	float3 halfway = normalize(normalizedLightVector + surface.view);
	float attenuation = calculate_point_light_attenuation(unnormalizedLightVector, entity.get_attenuation_radius());
	float3 radiance = entity.get_color().xyz * attenuation;
	return surface.calculate(normalizedLightVector, halfway, radiance);
}

inline float3 calculate_spot_light(in RendererEntity entity, in SurfaceDesc surface)
{
	float3 unnormalizedLightVector = entity.location - surface.location;
	float3 normalizedLightVector = normalize(unnormalizedLightVector);
	float3 halfway = normalize(normalizedLightVector + surface.view);
	// THINK WHAT TO DO WITH THE FACTOR
	float factor = dot(normalizedLightVector, entity.get_direction());
	float attenuation = calculate_spot_light_attenuation(
	   entity.get_direction(),
	   unnormalizedLightVector,
	   entity.get_attenuation_radius(),
	   entity.get_angle_scale(),
	   entity.get_angle_offset());
	float3 radiance = entity.get_color().xyz * attenuation;
	return surface.calculate(normalizedLightVector, halfway, radiance);
}

inline float3 calculate_directional_light(in RendererEntity entity, in SurfaceDesc surface)
{
	float3 lightVector = -entity.get_direction();
	float3 halfway = normalize(lightVector + surface.view);
	float4 lightColor = entity.get_color();
	float3 radiance = float3(lightColor.x, lightColor.y, lightColor.z);
	return surface.calculate(lightVector, halfway, radiance);
}

#endif // LIGHTING_HFw
