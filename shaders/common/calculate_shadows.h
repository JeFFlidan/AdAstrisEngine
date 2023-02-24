const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

float calculateDirLightShadow(in texture2D dirShadowMap, DirectionLight dirLight, sampler shadowSamp, vec3 fragPos)
{
	vec4 fragPosLightSpace = biasMat * dirLight.lightSpaceMat * vec4(fragPos, 1.0);
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	float currentDepth = projCoords.z;

	vec3 L = normalize(vec3(-dirLight.direction));
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(sampler2D(dirShadowMap, shadowSamp), 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			vec2 tempTexCoord = projCoords.xy + vec2(x, y) * texelSize;
			float pcfDepth = texture(sampler2D(dirShadowMap, shadowSamp), tempTexCoord).r;
			shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
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

float calculatePointLightShadow(PointLight pointLight, textureCube shadowMap, sampler samp, vec3 fragPos)
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
		if (currentDepth > closestDepth)
			shadow += 1.0;
	}
	shadow /= float(samples);

	return shadow;
}

float calculateSpotLightShadow(SpotLight spotLight, texture2D shadowMap, sampler samp, vec3 fragPos)
{
	vec4 fragPosLightSpace = biasMat * spotLight.lightSpaceMat * vec4(fragPos, 1.0);
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	//projCoords.xy = projCoords.xy * 0.5 + 0.5;

	float currentDepth = projCoords.z;
	
	float bias = 0.0035;
	float shadow = 0.0;

	float closestDepth = texture(nonuniformEXT(sampler2D(shadowMap, samp)), projCoords.xy).r;

	shadow = currentDepth > closestDepth ? 1.0 : 0.0;

	return shadow;
}