struct DirectionLight
{
	vec4 direction;
	vec4 colorAndIntensity;	// w = intensity
	mat4 lightViewMat;
	mat4 lightProjMat;
};

struct PointLight
{
	vec4 colorAndIntensity;
	vec4 positionAndAttRadius;
	float sourceRadius;
};

struct SpotLight
{
	vec4 colorAndIntensity;
	vec4 positionAndDistance;
	vec4 spotDirAndInnerConeRadius;
	float outerConeRadius;
};

