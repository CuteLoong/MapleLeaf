#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

layout(set=0, binding=1) uniform UniformHBAOData {
	uvec2 noiseScale;
    uint  numRays;
    uint  stepCount;
    float maxRadiusPixels;
    float sampleRadius;
    float pixelRadius;
    float intensity;
    float angleBias;
} hbaoData;

layout(set=0, binding = 2) uniform sampler2D inDepth;
layout(set=0, binding = 3) uniform sampler2D inStereoMask;
layout(set=0, binding = 4) uniform sampler2D inStereoMV;
layout(set=0, binding = 5) uniform sampler2D hbaoNoise;

#include <Misc/Constants.glsl>
#include <Misc/Camera.glsl>
#include <Misc/Utils.glsl>

layout(location = 0) out vec4 outColour;

layout(location = 0) in vec2 inUV;

vec2 RotateDirection(vec2 dir, vec2 cosSin)
{
  return vec2(dir.x*cosSin.x - dir.y*cosSin.y, dir.x*cosSin.y + dir.y*cosSin.x);
}

float ComputeAO(vec3 viewPosition, vec3 viewNormal, vec3 sampleViewPos, inout float topOcclusion)
{
    vec3 horizonVector = sampleViewPos - viewPosition;
    float horizonVectorLength = length(horizonVector);

    float occlusion = dot(viewNormal, horizonVector) / horizonVectorLength;

    float diff = max(occlusion - topOcclusion, 0.0f);
    topOcclusion = max(occlusion, topOcclusion);

    float distanceFactor = horizonVectorLength / hbaoData.sampleRadius;
    distanceFactor = clamp(1.0f - distanceFactor * distanceFactor, 0.0f, 1.0f);

    return diff * distanceFactor;
}

vec2 ConvertStereoUVToScreenUV(vec2 stereoUV, int viewIndex)
{
    vec2 screenUV;
    screenUV.x = stereoUV.x / 2.0f + float(viewIndex) * 0.5f;
    screenUV.y = stereoUV.y;
    return screenUV;
}

vec2 GetOtherEyeUV(vec2 screenUV)
{
    vec2 otherEyeUV = screenUV + texture(inStereoMV, screenUV).xy;
    return otherEyeUV;
}

int GetMask(vec2 screenUV)
{
    return int(texture(inStereoMask, screenUV).x);
}

void main()
{
    int viewIndex = inUV.x < 0.5f ? 0 : 1;
    int inverseViewIndex = 1 - viewIndex;
    
    if(viewIndex == 1) {
        outColour = vec4(1.0f);
        return;
    }
    
    vec2 uv = vec2(inUV.x, 1.0f - inUV.y);
    vec2 stereoUV =  vec2(uv.x * 2.0f - float(viewIndex), uv.y); // [0, 0.5] -> [0, 1] or [0.5, 1.0] -> [0, 1]
    int centerMask = GetMask(uv);

    vec3 viewPosition = StereoViewSpacePosAtScreenUV(stereoUV, viewIndex);
    vec3 viewNormal = StereoViewNormalAtScreenUVImproved(stereoUV, viewIndex);


    vec2 otherEyeUV;
    vec2 stereoUVOtherEye;

    vec3 viewPositionOtherEye;
    vec3 viewNormalOtherEye;

    if(centerMask == 0) {
        otherEyeUV = GetOtherEyeUV(uv);
        stereoUVOtherEye = vec2(otherEyeUV.x * 2.0f - float(inverseViewIndex), otherEyeUV.y);

        viewPositionOtherEye = StereoViewSpacePosAtScreenUV(stereoUVOtherEye, inverseViewIndex);
        viewNormalOtherEye = StereoViewNormalAtScreenUVImproved(stereoUVOtherEye, inverseViewIndex);
    }
    

    float stride = min(hbaoData.pixelRadius / viewPosition.z, hbaoData.maxRadiusPixels) / (hbaoData.stepCount + 1.0f);

    if(stride < 1.0f) {
        outColour = vec4(1.0f);
        return ;
    }

    float totalOcclusion = 0.0f;

    vec4 rand = texture(hbaoNoise, stereoUV * hbaoData.noiseScale).rgba;
    const float alpha = 2.0f * M_PI / hbaoData.numRays;

    for(int dirIndex = 0; dirIndex < hbaoData.numRays; dirIndex++) {
        float angle = alpha * float(dirIndex);

        vec2 direction = RotateDirection(vec2(cos(angle), sin(angle)), rand.xy);
        vec2 rayPixels = rand.zw * stride + 1.0f;
        float topOcclusion1 = hbaoData.angleBias;
        float topOcclusion2 = hbaoData.angleBias;

        for(int stepIndex = 0; stepIndex < hbaoData.stepCount; stepIndex++) {
            vec2 SnappedUV = round(rayPixels * direction) * camera.stereoPixelSize.zw + stereoUV; // calculate the pixel position in stereo space [0, 1]
            vec3 sampleViewPos = StereoViewSpacePosAtScreenUV(SnappedUV, viewIndex);

            if(centerMask == 0) {
                vec2 SnappedScreenUV = ConvertStereoUVToScreenUV(SnappedUV, viewIndex);
                int sampleMask = GetMask(SnappedScreenUV);
                
                vec2 OtherEyeSnappedScreenUV = GetOtherEyeUV(SnappedScreenUV);
                vec2 OtherEyeSnappedUV = vec2(OtherEyeSnappedScreenUV.x * 2.0f - float(inverseViewIndex), OtherEyeSnappedScreenUV.y);

                vec3 sampleOtherEyeViewPos = StereoViewSpacePosAtScreenUV(OtherEyeSnappedUV, inverseViewIndex);

                float occlusionCurEye = ComputeAO(viewPosition, viewNormal, sampleViewPos, topOcclusion1);
                float occlusionOtherEye = ComputeAO(viewPositionOtherEye, viewNormalOtherEye, sampleOtherEyeViewPos, topOcclusion2);

                totalOcclusion += (sampleMask == 1) ? max(occlusionCurEye, occlusionOtherEye) : ((occlusionCurEye + occlusionOtherEye) / 2.0f);
            }

            if(centerMask == 1) totalOcclusion += ComputeAO(viewPosition, viewNormal, sampleViewPos, topOcclusion1);

            rayPixels += stride;
        }
    }
    float weight = 1.0f / hbaoData.numRays;

    float ao = clamp(1.0f - totalOcclusion * weight, 0.0f, 1.0f);
    outColour = vec4(ao, ao, ao, 1.0f);
}