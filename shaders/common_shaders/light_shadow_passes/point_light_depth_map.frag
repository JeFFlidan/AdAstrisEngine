#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 lightPosition;
layout(location = 2) in float farPlane;

void main()
{
	float lightDistance = length(fragPos - lightPosition);
	lightDistance = lightDistance / farPlane;
	gl_FragDepth = lightDistance;
}
