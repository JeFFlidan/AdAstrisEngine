#version 440

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec4 outFragColor;

void main()
{
	outFragColor = vec4(1.f, 0.f, 1.f, 1.f);
}
