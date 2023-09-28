#ifndef SURFACE_HF
#define SURFACE_HF

#include "common.hlsli"

float distribution_ggx(in float3 normal, in float3 halfway, in float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(normal, halfway), 0.0);
	float sqrNdotH = NdotH * NdotH;

	float denominator = sqrNdotH * (a2 - 1.0f) + 1.0f;
	denominator = PI * denominator * denominator;

	return a2 / denominator;
}

float geometry_schlick_ggx(in float dotProduct, in float roughness)
{
	float k = pow(roughness + 1.0f, 2.0f) / 8.0f;
	float denominator = dotProduct * (1.0f - k) + k;
	return dotProduct / denominator;
}

float geometry_smith(in float3 normal, in float3 view, in float3 lightVector, float roughness)
{
	float NdotL = max(dot(normal, lightVector), 0.0f);
	float NdotV = max(dot(normal, view), 0.0f);

	float geomLight = geometry_schlick_ggx(NdotL, roughness);
	float geomView = geometry_schlick_ggx(NdotV, roughness);

	return geomLight * geomView;
}

float3 fresnel_schlick(in float cosTheta, in float3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0f);
}

float3 calculate_f0(in float3 albedo, in float metallic)
{
	float3 F0 = float3(0.04f, 0.04f, 0.04f);
	return lerp(F0, albedo, metallic);
}

struct SurfaceDesc
{
	// Input data. Must be set manually in pixel shader
	float3 location;
	float3 normal;
	float3 view;

	float4 albedo;
	float roughness;
	float metallic;
	float ao;

	float3 calculate(in float3 normalizedLightVector, in float3 halfwayVector, in float3 radiance)
	{
		float3 F0 = calculate_f0(albedo.xyz, metallic);
		
		float NDF = distribution_ggx(normal, halfwayVector, roughness);
		float G = geometry_smith(normal, view, normalizedLightVector, roughness);
		float3 F = fresnel_schlick(clamp(dot(halfwayVector, view), 0.0, 1.0), F0);

		float3 kD = float3(1.0, 1.0, 1.0) - F;
		kD *= 1.0 - metallic;

		float3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(normal, view), 0.0) * max(dot(normal, normalizedLightVector), 0.0) + 0.0001;
		float3 specular = numerator / denominator;

		float NdotL = max(dot(normal, normalizedLightVector), 0.0);
		return (kD * albedo.xyz / PI + specular) * radiance * NdotL + ao * 0.2 * albedo.rgb;
	}
};

#endif // SURFACE_HF
