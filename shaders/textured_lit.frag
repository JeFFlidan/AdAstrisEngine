#version 460

#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require

#include "data.h"

layout(location = 0) in vec2 texCoord;
layout(location = 1) in flat uint id;
layout(location = 2) in vec3 viewPos;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in mat3 TBN;

layout(location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) buffer DirectionLights
{
	DirectionLight casters[];
} dirLights;

layout(set = 0, binding = 1) buffer PointLights
{
	PointLight casters[];
} pointLights;

layout(set = 0, binding = 2) buffer SpotLights
{
	SpotLight casters[];
} spotLights;

layout(set = 0, binding = 5) uniform sampler samp;
layout(set = 0, binding = 6) uniform sampler shadowSamp;

layout(set = 0, binding = 4) uniform SceneData
{
	uint dirLightsAmount;
	uint pointLightsAmount;
	uint spotLightsAmount;
} sceneData;

layout(set = 2, binding = 0) uniform texture2D baseColorTextures[];
layout(set = 3, binding = 0) uniform texture2D normalTextures[];
layout(set = 4, binding = 0) uniform texture2D armTextures[];
layout(set = 5, binding = 0) uniform texture2D dirShadowMaps[];
layout(set = 6, binding = 0) uniform textureCube pointShadowMaps[];
layout(set = 7, binding = 0) uniform texture2D spotShadowMaps[];

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec4 calculateBRDF(vec3 N, vec3 H, vec3 V, vec3 L, vec3 F0, vec2 roughMetal);	// xyz - specular, w - kD, refracted light

float smoothDistanceAttr(float squaredDistance, float invSqrAttRadius);
float getDistanceAtt(vec3 unormalizedLightVector, float invSqrAttRadius);
float getAngleAtt(SpotLight spotLight, vec3 normalizedLightVector);
float calculatePointLightAttenuation(PointLight pointLight, vec3 unormalizedLightVector);
float calculateSpotLightAttenuation(SpotLight spotLight, vec3 unormalizedLightVector, vec3 normalizedLightVector);

vec3 calculateDirectionLight(vec3 F0, vec3 N, vec3 V, DirectionLight dirLight, vec3 albedo, float roughness, float metallic);
vec3 calculatePointLight(vec3 F0, vec3 N, vec3 V, PointLight pointLight, vec3 albedo, float roughness, float metallic);
vec3 calculateSpotLight(vec3 F0, vec3 N, vec3 V, SpotLight spotLight, vec3 albedo, float roughness, float metallic);

float calculateDirLightShadow(DirectionLight dirLight, vec3 N, int id);
float calculatePointLightShadow(PointLight pointLight, vec3 N, int id);
float calculateSpotLightShadow(SpotLight spotLight, vec3 N, int id);

void main()
{
	vec3 albedo = texture(nonuniformEXT(sampler2D(baseColorTextures[id], samp)), texCoord).xyz;
	albedo = pow(albedo, vec3(2.2));
	vec3 normal = texture(nonuniformEXT(sampler2D(normalTextures[id], samp)), texCoord).xyz;
	normal = normal * 2.0 - 1.0;
	normal = normalize(TBN * normal);
	vec3 arm = texture(nonuniformEXT(sampler2D(armTextures[id], samp)), texCoord).xyz;
	float ao = arm.r;
	float roughness = arm.g;
	float metallic = arm.b;

	float test = 0.0;

	vec3 view = normalize(viewPos - fragPos);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	vec3 dirLightsL0 = vec3(0.0);
	vec3 pointLightsL0 = vec3(0.0);
	vec3 spotLightsL0 = vec3(0.0);
	float dirShadow = 0.0;
	float pointShadow = 0.0;
	float spotShadow = 0.0;
	
	for (int i = 0; i != sceneData.dirLightsAmount; ++i)
	{
		dirLightsL0 += calculateDirectionLight(F0, normal, view, dirLights.casters[i], albedo, roughness, metallic);
		dirShadow = calculateDirLightShadow(dirLights.casters[i], normal, i);
	}

	dirLightsL0 *= (1.0 - dirShadow);
	dirLightsL0 *= 0.0;

	for (int i = 0; i != sceneData.pointLightsAmount; ++i)
	{
		pointLightsL0 += calculatePointLight(F0, normal, view, pointLights.casters[i], albedo, roughness, metallic);
		pointShadow = calculatePointLightShadow(pointLights.casters[i], normal, i);
	}

	pointLightsL0 *= (1.0 - pointShadow);

	for (int i = 0; i != sceneData.spotLightsAmount; ++i)
	{
		spotLightsL0 += calculateSpotLight(F0, normal, view, spotLights.casters[i], albedo, roughness, metallic);
		spotShadow = calculateSpotLightShadow(spotLights.casters[i], normal, i);
	}

	spotLightsL0 *= (1.0 - spotShadow);

	vec3 ambient = vec3(0.01) * albedo * ao;
	vec3 finalColor = ambient + (spotLightsL0 + dirLightsL0 + pointLightsL0);
	//finalColor = ambient + (spotLightsL0 + dirLightsL0 + pointLightsL0);

	finalColor = pow(finalColor, vec3(1.0/2.2));

	outFragColor = vec4(finalColor, 1.0);
	//float temp = calculateDirLightShadow(dirLights.casters[0], 0);
	//outFragColor = vec4(temp);
}

vec3 calculateDirectionLight(vec3 F0, vec3 N, vec3 V, DirectionLight dirLight, vec3 albedo, float roughness, float metallic)
{
	vec3 L = normalize(vec3(-dirLight.direction));
	vec3 H = normalize(V + L);
	vec3 radiance = vec3(dirLight.colorAndIntensity) * dirLight.colorAndIntensity.w;

	vec4 brdf = calculateBRDF(N, H, V, L, F0, vec2(roughness, metallic));

	float kD = brdf.w;
	vec3 specular = brdf.xyz;
	
	float NdotL = max(dot(N, L), 0.0);

	return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 calculatePointLight(vec3 F0, vec3 N, vec3 V, PointLight pointLight, vec3 albedo, float roughness, float metallic)
{
	vec3 unormalizedLightVector = vec3(pointLight.positionAndAttRadius.xyz) - fragPos;
	vec3 L = normalize(unormalizedLightVector);
	vec3 H = normalize(L + V);

	float attenuation = calculatePointLightAttenuation(pointLight, unormalizedLightVector);
	vec3 lightColor = pointLight.colorAndIntensity.xyz * (pointLight.colorAndIntensity.w / (4.0 * PI));
	vec3 radiance = lightColor * attenuation;

	vec4 brdf = calculateBRDF(N, H, V, L, F0, vec2(roughness, metallic));

	float kD = brdf.w;
	vec3 specular = brdf.xyz;
	
	float NdotL = max(dot(N, L), 0.0);

	return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 calculateSpotLight(vec3 F0, vec3 N, vec3 V, SpotLight spotLight, vec3 albedo, float roughness, float metallic)
{
	vec3 unormalizedLightVector = vec3(spotLight.positionAndDistance.xyz) - fragPos;
	vec3 L = normalize(unormalizedLightVector);
	vec3 H = normalize(L + V);

	float attenuation = calculateSpotLightAttenuation(spotLight, unormalizedLightVector, L);
	vec3 lightColor = spotLight.colorAndIntensity.xyz * (spotLight.colorAndIntensity.w / PI);
	vec3 radiance = lightColor * attenuation;

	vec4 brdf = calculateBRDF(N, H, V, L, F0, vec2(roughness, metallic));

	float kD = brdf.w;
	vec3 specular = brdf.xyz;
	
	float NdotL = max(dot(N, L), 0.0);

	return (kD * albedo / PI + specular) * radiance * NdotL;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float numerator = a2;
	float denominator = NdotH2 * (a2 - 1.0) + 1.0;
	denominator = PI * denominator * denominator;

	return numerator / denominator;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float temp = roughness + 1.0;
	float k = (temp * temp) / 8.0;

	float denominator = NdotV * (1.0 - k) + k;

	return NdotV / denominator;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);

	float Glight = GeometrySchlickGGX(NdotL, roughness);
	float Gview = GeometrySchlickGGX(NdotV, roughness);

	return Glight * Gview;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec4 calculateBRDF(vec3 N, vec3 H, vec3 V, vec3 L, vec3 F0, vec2 roughMetal)
{
	float roughness = roughMetal.r;
	float metallic = roughMetal.g;

	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
	vec3 specular = numerator / denominator;

	return vec4(specular, kD);
}

float smoothDistanceAttr(float squaredDistance, float invSqrAttRadius)
{
	float factor = squaredDistance * invSqrAttRadius;
	float smoothFactor = clamp(1.0 - pow(factor, 4.0), 0.0, 1.0);
	return smoothFactor * smoothFactor;
}

float getDistanceAtt(vec3 unormalizedLightVector, float invSqrAttRadius)
{
	float sqrDist = dot(unormalizedLightVector, unormalizedLightVector);
	float attenuation = 1.0 / (max(sqrDist, 0.01 * 0.01));
	attenuation *= smoothDistanceAttr(sqrDist, invSqrAttRadius);
	return attenuation;
}

float calculatePointLightAttenuation(PointLight pointLight, vec3 unormalizedLightVector)
{
	float attenuation = 1.0;
	float lightInvSqrAttRadius = pow(1.0 / pointLight.positionAndAttRadius.w, 2.0);
	attenuation *= getDistanceAtt(unormalizedLightVector, lightInvSqrAttRadius);

	return attenuation;
}

float getAngleAtt(SpotLight spotLight, vec3 normalizedLightVector)
{
	float spotScale = 1.0f / max(0.001, spotLight.spotDirAndInnerConeRadius.w - spotLight.outerConeRadius);
	float spotOffset = -spotLight.outerConeRadius * spotScale;
	vec3 dir = normalize(vec3(spotLight.spotDirAndInnerConeRadius.xyz));
	float cd = dot(-dir, normalizedLightVector);
	float attenuation = clamp(cd * spotScale + spotOffset, 0.0, 1.0);
	attenuation *= attenuation;
	return attenuation;
}

float calculateSpotLightAttenuation(SpotLight spotLight, vec3 unormalizedLightVector, vec3 normalizedLightVector)
{
	float attenuation = 1.0;
	float lightInvSqrMaxDist = pow(1.0 / spotLight.positionAndDistance.w, 2.0);
	attenuation *= getDistanceAtt(unormalizedLightVector, lightInvSqrMaxDist);
	attenuation *= getAngleAtt(spotLight, normalizedLightVector);
	return attenuation;
}

float calculateDirLightShadow(DirectionLight dirLight, vec3 N, int id)
{
	vec4 fragPosLightSpace = dirLight.lightSpaceMat * vec4(fragPos, 1.0);
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords.xy = projCoords.xy * 0.5 + 0.5;
	float currentDepth = projCoords.z;
	
	if (currentDepth > 1.0)
		return 0.0;

	vec3 L = normalize(vec3(-dirLight.direction));
	//float bias = max(0.05 * (1.0 - dot(N, L)), 0.005);
	float bias = 0.0035;	
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(sampler2D(dirShadowMaps[id], shadowSamp), 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			vec2 tempTexCoord = projCoords.xy + vec2(x, y) * texelSize;
			float pcfDepth = texture(nonuniformEXT(sampler2D(dirShadowMaps[id], shadowSamp)), tempTexCoord).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	shadow /= 9.0;
	
	return shadow;
}

vec3 sampleOffsetDirections[20] = vec3[]
(
	vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
	vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
	vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
	vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
	vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);  

float calculatePointLightShadow(PointLight pointLight, vec3 N, int id)
{
	vec3 lightToFrag = fragPos - vec3(pointLight.positionAndAttRadius);
	float currentDepth = length(lightToFrag);
	float bias = 0.05;
	float shadow = 0.0;
	int samples = 20;
	float distRadius = 0.05;
	for (int i = 0; i != samples; ++i)
	{
		vec3 coords = lightToFrag + sampleOffsetDirections[i] * distRadius;
		float closestDepth = texture(nonuniformEXT(samplerCube(pointShadowMaps[id], shadowSamp)), coords).r;
		closestDepth *= pointLight.farPlane;
		if (currentDepth - bias > closestDepth)
			shadow += 1.0;
	}
	shadow /= float(samples);
	return shadow;
}

float calculateSpotLightShadow(SpotLight spotLight, vec3 N, int id)
{
	vec4 fragPosLightSpace = spotLight.lightSpaceMat * vec4(fragPos, 1.0);
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords.xy = projCoords.xy * 0.5 + 0.5;
	float currentDepth = projCoords.z;
	
	if (currentDepth > 1.0)
		return 0.0;

	//vec3 L = normalize(vec3(-dirLight.direction));
	//float bias = max(0.05 * (1.0 - dot(N, L)), 0.005);
	float bias = 0.00035;
	float shadow = 0.0;

	float pcfDepth = texture(nonuniformEXT(sampler2D(spotShadowMaps[id], shadowSamp)), projCoords.xy).r;
			
	shadow = currentDepth - bias > pcfDepth ? 1.0 : 0.0;

	return shadow;
}

