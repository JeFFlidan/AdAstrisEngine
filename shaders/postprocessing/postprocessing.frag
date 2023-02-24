#version 460

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

//layout(set = 0, binding = 0) uniform sampler2D opaqueColorAttach;
//layout(set = 0, binding = 1) uniform sampler2D transparencyColorAttach;
//layout(set = 0, binding = 2) uniform sampler2D opaqueDepthAttach;
//layout(set = 0, binding = 3) uniform sampler2D transparencyDepthAttach;

layout(set = 0, binding = 0) uniform sampler2D finalAttach;

const float offset = 1.0 / 300.0;

void main()
{
	// vec2 offsets[9] = vec2[](
	// 	vec2(-offset,  offset), // top-left
	// 	vec2( 0.0f,    offset), // top-center
	// 	vec2( offset,  offset), // top-right
	// 	vec2(-offset,  0.0f),   // center-left
	// 	vec2( 0.0f,    0.0f),   // center-center
	// 	vec2( offset,  0.0f),   // center-right
	// 	vec2(-offset, -offset), // bottom-left
	// 	vec2( 0.0f,   -offset), // bottom-center
	// 	vec2( offset, -offset)  // bottom-right    
	// );

	// float kernel[9] = float[](
	// 	-1, -1, -1,
	// 	-1,  9, -1,
	// 	-1, -1, -1
	// );

	// vec3 sampleTex[9];
	// for (int i = 0; i != 9; ++i)
	// {
	// 	sampleTex[i] = vec3(texture(screenTexture, texCoord.st + offsets[i]));
	// }

	// vec3 color = vec3(0.0);
	// for (int i = 0; i != 9; ++i)
	// {
	// 	color += sampleTex[i] * kernel[i];
	// }

	//fragColor = texture(screenTexture, texCoord);
	//float average = 0.2126 * fragColor.r + 0.7152 * fragColor.g + 0.0722 * fragColor.b;

	//fragColor = vec4(vec3(average), 1.0);
	//vec4 opaqueColor = pow(texture(opaqueColorAttach, texCoord), vec4(2.2));

	//fragColor = composeTransparencyAndOpaque();

	vec3 color = texture(finalAttach, texCoord).xyz;
	color = pow(color, vec3(2.2));

	fragColor = vec4(color, 1.0);

	//float average = 0.2126 * fragColor.r + 0.7152 * fragColor.g + 0.0722 * fragColor.b;
	//fragColor = vec4(vec3(average), 1.0);
	//fragColor = texture(transparencyColorAttach, texCoord);
	//fragColor = texture(opaqueColorAttach, texCoord);

	//fragColor = vec4(0.3f);
}