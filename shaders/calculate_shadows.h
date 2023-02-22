float calculateDirLightShadow(DirectionLight dirLight, texture2D shadowMap, sampler2D samp, vec3 fragPos)
{
	vec4 fragPosLightSpace = dirLight.lightSpaceMat * vec4(fragPos, 1.0);
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords.xy = projCoords.xy * 0.5 + 0.5;
	float currentDepth = projCoords.z;
	
	if (currentDepth > 1.0)
		return 0.0;

	vec3 L = normalize(vec3(-dirLight.direction));
	float bias = 0.0035;	
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(sampler2D(shadowMap, samp), 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			vec2 tempTexCoord = projCoords.xy + vec2(x, y) * texelSize;
			float pcfDepth = texture(nonuniformEXT(sampler2D(shadowMap, samp)), tempTexCoord).r;
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

float calculatePointLightShadow(PointLight pointLight, textureCube shadowMap, samlerCube samp, vec3 fragPos)
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
		float closestDepth = texture(nonuniformEXT(samplerCube(shadowMap, samp)), coords).r;
		closestDepth *= pointLight.farPlane;
		if (currentDepth - bias > closestDepth)
			shadow += 1.0;
	}
	shadow /= float(samples);

	return shadow;
}

float calculateSpotLightShadow(SpotLight spotLight, texture2D shadowMap, sampler2D samp, vec3 fragPos)
{
	vec4 fragPosLightSpace = spotLight.lightSpaceMat * vec4(fragPos, 1.0);
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	projCoords.xy = projCoords.xy * 0.5 + 0.5;

	float currentDepth = projCoords.z;
	
	float bias = 0.00035;
	float shadow = 0.0;

	float closestDepth = texture(nonuniformEXT(sampler2D(shadowMap, samp)), projCoords.xy).r;

	shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	return shadow;
}
