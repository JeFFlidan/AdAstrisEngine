#version 450

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragVelocity;

layout(set = 0, binding = 0) uniform texture2D opaqueColorAttach;
layout(set = 0, binding = 1) uniform texture2D transparencyColorAttach;
layout(set = 0, binding = 2) uniform texture2D opaqueVelocityAttach; 
layout(set = 0, binding = 3) uniform texture2D transparencyVelocityAttach;
layout(set = 0, binding = 4) uniform texture2D opaqueDepthAttach;
layout(set = 0, binding = 5) uniform texture2D transparencyDepthAttach;
layout(set = 0, binding = 6) uniform sampler nearestSamp;

void composeTransparencyAndOpaque(out vec4 color, out vec2 velocity);

void main()
{
    vec2 velocity;
    vec4 color;
    
    composeTransparencyAndOpaque(color, velocity);
    
    fragVelocity = velocity;
    fragColor = color;
}

void composeTransparencyAndOpaque(out vec4 color, out vec2 velocity)
{
    ivec2 attachTexCoord = ivec2(gl_FragCoord.xy);
    float opaqueDepth = texelFetch(sampler2D(opaqueDepthAttach, nearestSamp), attachTexCoord, 0).r;
    float transparencyDepth = texelFetch(sampler2D(transparencyDepthAttach, nearestSamp), attachTexCoord, 0).r;

    if (opaqueDepth < transparencyDepth)
    {
        color = texelFetch(sampler2D(opaqueColorAttach, nearestSamp), attachTexCoord, 0);
        velocity = texelFetch(sampler2D(opaqueVelocityAttach, nearestSamp), attachTexCoord, 0).xy;
    }
    else
    {
        color = texelFetch(sampler2D(transparencyColorAttach, nearestSamp), attachTexCoord, 0);
        velocity = texelFetch(sampler2D(transparencyVelocityAttach, nearestSamp), attachTexCoord, 0).xy;
    }
}

