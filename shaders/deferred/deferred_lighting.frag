#version 460

#extension GL_EXT_nonuniform_qualifier : require

#include <shaders/common/data.h>
#include <shaders/common/PBR_functions.h>
#include <shaders/common/calculate_shadows.h>

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform SceneData
{
	uint dirLightsAmount;
	uint pointLightsAmount;
	uint spotLightsAmount;
} sceneData;

layout(set = 0, binding = 1) uniform CameraBuffer
{
	CameraData cameraBuffer;
};

layout(set = 0, binding = 2) buffer DirectionalLights
{
	DirectionLight casters[];
} dirLights;

layout(set = 0, binding = 3) buffer PointLights
{
	PointLight casters[];
} pointLights;

layout(set = 0, binding = 4) buffer SpotLights
{
	SpotLight casters[];
} spotLights;

layout(set = 0, binding = 5) uniform texture2D gAlbedo;
layout(set = 0, binding = 6) uniform texture2D gNormal;
layout(set = 0, binding = 7) uniform texture2D gSurface;
layout(set = 0, binding = 8) uniform texture2D gDepth;
layout(set = 0, binding = 9) uniform sampler attachSamp;
layout(set = 0, binding = 10) uniform sampler shadowSamp;

layout(set = 1, binding = 0) uniform texture2D dirShadowMaps[];
layout(set = 2, binding = 0) uniform textureCube pointShadowMaps[];
layout(set = 3, binding = 0) uniform texture2D spotShadowMaps[];

vec3 getWorldPosition(float depth);

void main()
{
	float depth = texture(sampler2D(gDepth, attachSamp), texCoord).r;
	vec3 fragPos = getWorldPosition(depth);
	vec3 albedo = texture(sampler2D(gAlbedo, attachSamp), texCoord).xyz;
	albedo = pow(albedo, vec3(2.2));
	vec3 normal = texture(sampler2D(gNormal, attachSamp), texCoord).xyz;
	vec3 arm = texture(sampler2D(gSurface, attachSamp), texCoord).xyz;
	float ao = arm.r;
	float roughness = arm.g;
	float metallic = arm.b;

	vec3 view = normalize(vec3(cameraBuffer.cameraPosition) - fragPos);

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
		DirectionLight dirLight = dirLights.casters[i];
		if (dirLight.isVisible != 0)
		{
			dirLightsL0 += calculateDirectionLight(F0, normal, view, dirLight, albedo, roughness, metallic);
			if (dirLight.castShadows != 0)
			{
				dirShadow = calculateDirLightShadow(dirShadowMaps[nonuniformEXT(i)], dirLight, shadowSamp, fragPos);
			}
		}
	}

	dirLightsL0 *= (1.0 - dirShadow);

	vec2 roughMetal = vec2(roughness, metallic);

	for (int i = 0; i != sceneData.pointLightsAmount; ++i)
	{
		PointLight pointLight = pointLights.casters[i];
		if (pointLight.isVisible != 0)
		{
			pointLightsL0 += calculatePointLight(F0, fragPos, normal, view, pointLight, albedo, roughMetal);
			if (pointLight.castShadows != 0)
			{
				pointShadow = calculatePointLightShadow(pointLights.casters[i], pointShadowMaps[nonuniformEXT(i)], shadowSamp, fragPos);
			}
		}
	}

	pointLightsL0 *= (1.0 - pointShadow);

	for (int i = 0; i != sceneData.spotLightsAmount; ++i)
	{
		SpotLight spotLight = spotLights.casters[i];
		if (spotLight.isVisible != 0)
		{
			spotLightsL0 += calculateSpotLight(F0, fragPos, normal, view, spotLight, albedo, roughMetal);
			if (spotLight.castShadows != 0)
			{
				spotShadow = calculateSpotLightShadow(spotLights.casters[i], spotShadowMaps[nonuniformEXT(i)], shadowSamp, fragPos);
			}
		}
	}

	spotLightsL0 *= (1.0 - spotShadow);

	vec3 ambient = vec3(0.01) * albedo * ao;
	vec3 finalColor = ambient + (spotLightsL0 + dirLightsL0 + pointLightsL0);

	finalColor = pow(finalColor, vec3(1.0/2.2));

	fragColor = vec4(finalColor, 1.0);
}

vec3 getWorldPosition(float depth)
{
	vec4 clipSpacePos = vec4(texCoord * 2.0 - 1.0, depth, 1.0);

	vec4 worldSpacePos = cameraBuffer.invViewProj * clipSpacePos;

	worldSpacePos.xyz /= worldSpacePos.w;

	return vec3(worldSpacePos);
}
