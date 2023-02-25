#version 450

layout(location = 0) in vec3 data;

void main()
{
	vec4 newData = vec4(data, 1.0);
}
