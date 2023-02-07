#version 450

#extension GL_GOOGLE_include_directive : require
#include "data.h"

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform SettingsBuffer
{
    Settings settings;
};
layout(set = 0, binding = 1) uniform texture2D currentColorAttach;
layout(set = 0, binding = 2) uniform texture2D oldColorAttach;
layout(set = 0, binding = 3) uniform texture2D opaqueVelocityAttach;
layout(set = 0, binding = 4) uniform sampler linearSamp;
layout(set = 0, binding = 5) uniform sampler nearestSamp;

// Found this function in the project by aviktorov: https://github.com/aviktorov/scapes
void getNeighborsMinMax(in vec4 center, in vec2 uv, out vec4 colorMin, out vec4 colorMax)
{
    colorMin = center;
    colorMax = center;
    
    #define CalcNeighbor(offset) \
    { \
        vec4 neighbor = textureOffset(sampler2D(currentColorAttach, nearestSamp), uv, offset); \
        colorMin = min(colorMin, neighbor); \
        colorMax = max(colorMax, neighbor); \
    } \

    CalcNeighbor(ivec2(-1, 0));
    CalcNeighbor(ivec2(1, 0));
    CalcNeighbor(ivec2(0, -1));
    CalcNeighbor(ivec2(0, 1));
    CalcNeighbor(ivec2(-1, 1));
    CalcNeighbor(ivec2(1, 1));
    CalcNeighbor(ivec2(1, -1));
    CalcNeighbor(ivec2(1, 1));
}

void main()
{
    if (settings.isTaaEnabled == 1)
    {    
        vec2 motionVector = texture(sampler2D(opaqueVelocityAttach, nearestSamp), texCoord).xy;

         vec2 oldUv = texCoord + motionVector;
         vec4 curColor = texture(sampler2D(currentColorAttach, nearestSamp), texCoord);

         if (oldUv.x < 0.0 || oldUv.x > 1.0 || oldUv.y < 0.0 || oldUv.y > 1.0)
         {
             fragColor = curColor;
         }
         else
         {
             vec4 oldColor = texture(sampler2D(oldColorAttach, linearSamp), oldUv);

             vec4 colorMin;
             vec4 colorMax;

             getNeighborsMinMax(curColor, texCoord, colorMin, colorMax);

             oldColor = clamp(oldColor, colorMin, colorMax);
            
             // Have to think how to calculate alpha better
             float alpha = 1.0 / 15.0;

             fragColor = mix(oldColor, curColor, alpha);
         }
    }
    else
    {
        fragColor = texture(sampler2D(currentColorAttach, nearestSamp), texCoord);
    }
}