#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

#define M_PI 3.14159265f

layout(set=0, binding=0) uniform UniformScene
{
	mat4  projection;
	mat4  view;
    mat4  invProjection;
    mat4  invView;
    vec4  zBufferParams;
    vec4 pixelSize; // camera's pixelWidth, pixelHeight, 1.0 / pixelWidth, 1.0f / pixelHeight
	vec3  cameraPosition;
} scene;

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
layout(set=0, binding = 3) uniform sampler2D hbaoNoise;

layout(location = 0) out vec4 outColour;

layout(location = 0) in vec2 inUV;

float Linear01Depth(float z)
{
    return 1.0f / (scene.zBufferParams.x * z + scene.zBufferParams.y);
}

// inspired by keijiro's depth inverse projection
// https://github.com/keijiro/DepthInverseProjection
// constructs view space ray at the far clip plane from the screen uv
// then multiplies that ray by the linear 01 depth
vec3 ViewSpacePosAtScreenUV(vec2 uv)
{
    vec3 viewSpaceRay = vec3(scene.invProjection * (vec4(uv * 2.0f - 1.0f, 1.0f, 1.0f) * scene.zBufferParams.z));
    float rawDepth = texture(inDepth, uv).r;
    return viewSpaceRay * Linear01Depth(rawDepth);
}

vec3 ViewNormalAtScreenUV(vec2 uv)
{
    // get current pixel's view space position
    vec3 viewSpacePos_c = ViewSpacePosAtScreenUV(uv + vec2(0.0f, 0.0f));

    // get view space position at 1 pixel offsets in each major direction
    vec3 viewSpacePos_r = ViewSpacePosAtScreenUV(uv + vec2(scene.pixelSize.z, 0.0f));
    vec3 viewSpacePos_u = ViewSpacePosAtScreenUV(uv + vec2(0.0f, scene.pixelSize.w));

    // get the difference between the current and each offset position
    vec3 hDeriv = viewSpacePos_r - viewSpacePos_c;
    vec3 vDeriv = viewSpacePos_u - viewSpacePos_c;

    // get view space normal from the cross product of the diffs
    vec3 viewNormal = normalize(cross(vDeriv,  hDeriv));

    return viewNormal;
}

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

void main()
{
    vec2 uv = vec2(inUV.x, 1.0f - inUV.y);
    vec3 viewPosition = ViewSpacePosAtScreenUV(uv);
    vec3 viewNormal = ViewNormalAtScreenUV(uv);

    float stride = min(hbaoData.pixelRadius / viewPosition.z, hbaoData.maxRadiusPixels) / (hbaoData.stepCount + 1.0f);

    if(stride < 1.0f) {
        outColour = vec4(1.0f);
        return ;
    }

    float totalOcclusion = 0.0f;

    vec4 rand = texture(hbaoNoise, uv * hbaoData.noiseScale).rgba;
    const float alpha = 2.0f * M_PI / hbaoData.numRays;

    for(int dirIndex = 0; dirIndex < hbaoData.numRays; dirIndex++) {
        float angle = alpha * float(dirIndex);

        vec2 direction = RotateDirection(vec2(cos(angle), sin(angle)), rand.xy);
        vec2 rayPixels = rand.zw * stride + 1.0f;
        float topOcclusion = hbaoData.angleBias;

        for(int stepIndex = 0; stepIndex < hbaoData.stepCount; stepIndex++) {
            vec2 SnappedUV = round(rayPixels * direction) * scene.pixelSize.zw + uv;
            vec3 sampleViewPos = ViewSpacePosAtScreenUV(SnappedUV);

            rayPixels += stride;

            totalOcclusion += ComputeAO(viewPosition, viewNormal, sampleViewPos, topOcclusion);
        }
    }
    float weight = 1.0f / hbaoData.numRays;

    float ao = clamp(1.0f - totalOcclusion * weight, 0.0f, 1.0f);
    outColour = vec4(ao, ao, ao, 1.0f);
}