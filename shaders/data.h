struct DirectionLight
{
	vec4 direction;
	vec4 colorAndIntensity;	// w = intensity
	mat4 lightSpaceMat;
	int isVisible;
	int castShadows;
	int data1, data2;
};

struct PointLight
{
	vec4 colorAndIntensity;
	vec4 positionAndAttRadius;
	float sourceRadius;
	float farPlane;
	int isVisible;
	int castShadows;
	mat4 lightSpaceMat[6];
};

struct SpotLight
{
	vec4 colorAndIntensity;
	vec4 positionAndDistance;
	vec4 spotDirAndInnerConeRadius;
	mat4 lightSpaceMat;
	float outerConeRadius;
	int isVisible;
	int castShadows;
	float farPlane;
};

struct ObjectData
{
	mat4 model;
	vec4 radius;
	vec4 extents;
};

