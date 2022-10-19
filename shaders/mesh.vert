#version 460

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vColor;
layout(location = 3) in vec2 vTexCoord;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out int index;

layout(set = 0, binding = 0) uniform CameraBuffer
{
	mat4 view;
	mat4 proj;
	mat4 viewproj;
} cameraData;

struct ObjectData
{
	vec4 color;
	mat4 model;
};

layout(set = 1, binding = 0, std430) buffer ObjectBuffer
{
	ObjectData color[];
} objectBuffer;

void main()
{
	mat4 modelMatrix = objectBuffer.color[gl_BaseInstance].model;
	//modelMatrix = mat4(1.0);
	mat4 transformMatrix = (cameraData.viewproj * modelMatrix);
	gl_Position = transformMatrix * vec4(vPosition, 1.f);
	outColor = objectBuffer.color[gl_BaseInstance].color.xyz;
	texCoord = vTexCoord;
	index = gl_BaseInstance;
}
