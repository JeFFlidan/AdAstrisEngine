#version 440
layout(location = 0) in vec3 color;
layout(location = 1) in vec2 texCoord;
layout(location = 2) flat in int index;

layout(location = 0) out vec4 outFragColor;

void main()
{
    #ifdef RED_COLOR
	outFragColor = vec4(1.f, 0.f, 0.f, 1.f);
	#endif

	outFragColor = vec4(0.f, 0.f, 1.f, 1.f);
}
