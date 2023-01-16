struct DirectionLight
{
	vec4 direction;
	vec4 colorAndIntensity;	// w = intensity
	mat4 lightSpaceMat;
};

struct PointLight
{
	vec4 colorAndIntensity;
	vec4 positionAndAttRadius;
	float sourceRadius;
	float farPlane;
	float data1, data2;
	mat4 lightSpaceMat[6];
};

struct SpotLight
{
	vec4 colorAndIntensity;
	vec4 positionAndDistance;
	vec4 spotDirAndInnerConeRadius;
	mat4 lightSpaceMat;
	float outerConeRadius;
	float nearPlane;
	float farPlane;
	float data;
};

struct ObjectData
{
	mat4 model;
	vec4 radius;
	vec4 extents;
};

