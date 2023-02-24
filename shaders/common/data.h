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
	vec4 sphereBounds;
	vec4 extents;

	uint baseColorTexId;
	uint normalTexId;
	uint armTexId;
	uint data;
};

struct CameraData
{
	mat4 view;
	mat4 oldView;
	mat4 proj;
	mat4 viewProj;
	mat4 invViewProj;
	vec4 cameraPosition;
};

struct Node
{
	vec4 color;
	float depth;
	uint next;
};

struct TAAData
{
	vec4 haltonSequence[36];
	float haltonScale;
	uint numSamples;
};

struct Settings
{

	ivec2 viewportRes;
	uint totalFrames;
	uint isTaaEnabled;
	float taaAlpha;
	uint data1, data2, data3;
};