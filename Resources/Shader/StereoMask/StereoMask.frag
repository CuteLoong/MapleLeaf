#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

layout(set=0, binding = 1) uniform sampler2D inDepth;
layout(set=0, binding = 2) uniform sampler2D inPosition;
layout(set=0, binding = 3) uniform writeonly image2D OccluderMap;

#include <Misc/Constants.glsl>
#include <Misc/Camera.glsl>
#include <Misc/Utils.glsl>

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outMV;

void main()
{
    // float ipd = length(camera.cameraStereoPosition[1] - camera.cameraStereoPosition[0]);
    int viewIndex = inUV.x < 0.5f ? 0 : 1;
    int inverseViewIndex = 1 - viewIndex;

    vec2 uv = vec2(inUV.x, 1.0f - inUV.y);

    vec3 worldPosition = texture(inPosition, uv).rgb;
    vec4 otherEyeProj = camera.stereoProjection[inverseViewIndex] * camera.stereoView[inverseViewIndex] * vec4(worldPosition, 1.0f);
    otherEyeProj /= otherEyeProj.w;

    if( otherEyeProj.x > 1.0f || otherEyeProj.y > 1.0f || otherEyeProj.z > 1.0f || otherEyeProj.x < -1.0f || otherEyeProj.y < -1.0f || otherEyeProj.z < -1.0f)
    {
        outColour = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        outMV = vec4(0.0f, 0.0f, 1.0f, 1.0f);
        return;
    }

    vec2 otherEyeUV = vec2(otherEyeProj.x, -otherEyeProj.y) * 0.5f + 0.5f; 
    otherEyeUV = StereoUVToScreenUV(otherEyeUV, inverseViewIndex);

    float otherEyeDepth = texture(inDepth, otherEyeUV).r;
    float linear01DepthDiff = abs(Linear01Depth(otherEyeDepth) - Linear01Depth(otherEyeProj.z));

    int mask = linear01DepthDiff > 0.002f ? 1 : 0;
    // float parallax = (mask == 0) ? 2.0f * atan(ipd / (2.0f * LinearEyeDepth(otherEyeProj.z))) : 0.0f;

    vec2 motionVector = vec2(ivec2(otherEyeUV * camera.pixelSize.xy) - ivec2(uv * camera.pixelSize.xy)) * camera.pixelSize.zw; // in same texture space
    outColour = vec4(mask, mask, mask, 1.0f);
    outMV = vec4(motionVector, 1.0f, 1.0f);
    if(mask == 1) {
        imageStore(OccluderMap, ivec2(otherEyeUV * camera.pixelSize.xy), vec4(uv.x, uv.y, 0.0f, 0.0f));
    }
}