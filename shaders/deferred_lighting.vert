#version 460

#extension GL_GOOGLE_include_directive : require

#include "data.h"

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTexCoord;

layout(location = 0) out vec2 texCoord;

layout(set = 0, binding = 1) uniform CameraBuffer
{
	CameraData cameraBuffer;
};

/*vec3 getViewRay(vec3 pos)
{
	vec3 cRight = cameraBuffer.cameraRight.xyz;
	vec3 cUp = cameraBuffer.cameraUp.xyz;
	vec3 cForward = cameraBuffer.cameraFront.xyz;
	vec3 leftRight = cRight * pos.x * cameraBuffer.fovHorizontal;
	vec3 upDown = cUp * pos.y * cameraBuffer.fovVertical;
	vec3 forward = cForward;

	//return (forward + leftRight + upDown);
	return upDown;
}*/

void main()
{
	gl_Position = vec4(vPos, 1.0);
	texCoord = vTexCoord;
}
