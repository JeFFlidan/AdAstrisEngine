#version 460

#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 texCoord;
layout(location = 1) in flat uint id;
layout(location = 2) in vec3 viewPos;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in mat3 TBN;

layout(location = 0) out vec4 outFragColor;

/*layout(set = 0, binding = 1) uniform SceneData{
	vec4 fogColor; // w is for exponent
	vec4 fogDistances; //x for min, y for max, zw unused.
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
} sceneData;*/

struct DirectionLight
{
	vec4 direction;
	vec4 colorAndIntensity;	// w = intensity
};

struct PointLight
{
	vec4 color;
	vec4 position;
	float attenuationRadius;
	float sourceRadius;
	float intensity;
};

struct SpotLight
{
	vec4 color;
	vec4 position;
	float innerConeRadius;
	float outerConeRadius;
	float distance;
	float intensity;
};

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

layout(set = 0, binding = 4) uniform SceneData
{
	uint dirLightsAmount;
	uint pointLightsAmount;
	uint spotLightsAmount;
} sceneData;

layout(set = 2, binding = 0) uniform texture2D baseColorTextures[];
layout(set = 3, binding = 0) uniform texture2D normalTextures[];
layout(set = 4, binding = 0) uniform texture2D armTextures[];

//layout(set = 2, binding = 1) uniform sampler samp;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
float ShadowCalculation(vec4 fragPosLightSpace);
vec3 calculateDirectionLight(vec3 F0, vec3 N, vec3 V, DirectionLight dirLight, vec3 albedo, float roughness, float metallic);
vec3 calculatePointLight(vec3 F0, vec3 N, vec3 V, PointLight pointLight, vec3 albedo, float roughness, float metallic);
vec3 calculateSpotLight(vec3 F0, vec3 N, vec3 V, SpotLight spotLight, vec3 albedo, float roughness, float metallic);

float calculate_point_light_attenuation(PointLight pointLight);

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

	vec3 view = normalize(viewPos - fragPos);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	vec3 dirLightsL0 = vec3(0.0);
	vec3 pointLightsL0 = vec3(0.0);
	vec3 spotLightsL0 = vec3(0.0);
	
	for (int i = 0; i != sceneData.dirLightsAmount; ++i)
	{
		dirLightsL0 += calculateDirectionLight(F0, normal, view, dirLights.casters[i], albedo, roughness, metallic);
	}

	/*for (int i = 0; i != sceneData.pointLightsAmount; ++i)
	{
		pointLightsL0 += calculatePointLight(F0, normal, view, pointLights.casters[i], albedo, roughness, metallic);
	}*/

	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 finalColor = ambient + dirLightsL0;

	finalColor = pow(finalColor, vec3(1.0/2.2));

	vec4 test1 = pointLights.casters[0].color;
	vec4 test2 = spotLights.casters[0].color;
	
	outFragColor = vec4(finalColor, 1.0);
}

vec3 calculateDirectionLight(vec3 F0, vec3 N, vec3 V, DirectionLight dirLight, vec3 albedo, float roughness, float metallic)
{
	vec3 L = normalize(vec3(-dirLight.direction));
	vec3 H = normalize(V + L);
	vec3 radiance = vec3(dirLight.colorAndIntensity) * dirLight.colorAndIntensity.w;

	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
	vec3 specular = numerator / denominator;

	float NdotL = max(dot(N, L), 0.0);

	return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 calculatePointLight(vec3 F0, vec3 N, vec3 V, PointLight pointLight, vec3 albedo, float roughness, float metallic)
{
	// TODO

	/*vec3 L = normalize(vec3(pointLight.position) - fragPos);
	vec3 H = normalize(L + V);
	float dist = length(L);
	float attenuation = 
	vec3 radiance = vec3(pointLight.color) * pointLight.intensity */

	return vec3(1.0);
}

vec3 calculateSpotLight(vec3 F0, vec3 N, vec3 V, SpotLight spotLight, vec3 albedo, float roughness, float metallic)
{
	// TODO
	return vec3(1.0);
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

float calculate_point_light_attenuation(PointLight pointLight)
{
	// TODO
	return 0.0;
}

