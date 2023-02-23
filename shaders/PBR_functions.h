const float PI = 3.14159265359;

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
	//vec3 dir = normalize(15.0 * vec3(spotLight.spotDirAndInnerConeRadius.xyz));
	vec3 temp = vec3(spotLight.spotDirAndInnerConeRadius);
	vec3 dir;
	dir.x = cos(temp.y) * cos(temp.x);
	dir.y = sin(temp.x);
	dir.z = sin(temp.y) * cos(temp.x);
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

vec3 calculatePointLight(vec3 F0, vec3 fragPos, vec3 N, vec3 V, PointLight pointLight, vec3 albedo, vec2 roughMetal)
{
	float roughness = roughMetal.x;
	float metallic = roughMetal.y;
	
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

vec3 calculateSpotLight(vec3 F0, vec3 fragPos, vec3 N, vec3 V, SpotLight spotLight, vec3 albedo, vec2 roughtMetal)
{
	float roughness = roughtMetal.x;
	float metallic = roughtMetal.y;

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
