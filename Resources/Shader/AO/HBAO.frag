#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

#define M_PI 3.14159265f

layout(set=0, binding=0) uniform UniformScene
{
	mat4 projection;
	mat4 view;
    mat4 invProjection;
    mat4 invView;
    vec4 zBufferParams;
	vec3 cameraPosition;
} scene;

layout(set=0, binding=1) uniform UniformControl {
	uvec2 noiseScale;
    uvec2 screenSize;
    uint  numRays;
    float strengthPerRay;
    uint  maxStepsPerRay;
    float sampleRadius;
} control;

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
    vec3 viewSpacePos_r = ViewSpacePosAtScreenUV(uv + vec2(1.0f / control.screenSize.x, 0.0f));
    vec3 viewSpacePos_u = ViewSpacePosAtScreenUV(uv + vec2(0.0f, 1.0f / control.screenSize.y));

    // get the difference between the current and each offset position
    vec3 hDeriv = viewSpacePos_r - viewSpacePos_c;
    vec3 vDeriv = viewSpacePos_u - viewSpacePos_c;

    // get view space normal from the cross product of the diffs
    vec3 viewNormal = normalize(cross(hDeriv, vDeriv));

    return viewNormal;
}

vec2 RotateDirection(vec2 dir, vec2 cosSin)
{
  return vec2(dir.x*cosSin.x - dir.y*cosSin.y, dir.x*cosSin.y + dir.y*cosSin.x);
}

float ComputeAO(vec3 viewPosition, vec3 viewNormal, vec3 sampleViewPos)
{
    vec3 horizonVector = sampleViewPos - viewPosition;
    float horizonVectorLength = length(horizonVector);

    float occlusion;

    occlusion = dot(viewNormal, horizonVector) / horizonVectorLength;

    float distanceFactor = clamp(horizonVectorLength / control.sampleRadius, 0.0f, 1.0f);
    distanceFactor = 1.0f - distanceFactor * distanceFactor;

    return clamp(occlusion - 0.03f, 0.0f, 1.0f) * distanceFactor;
}

void main()
{
    vec2 uv = vec2(inUV.x, 1.0f - inUV.y);
    vec3 viewPosition = ViewSpacePosAtScreenUV(uv);
    vec3 viewNormal = ViewNormalAtScreenUV(uv);

    // get projected sphere size.
    float w = viewPosition.z * abs(scene.projection[2][3]) + abs(scene.projection[3][3]);
    vec2 projectedRadii = control.sampleRadius * vec2(scene.projection[0][0], scene.projection[1][1]) / (2.0f * w);
    float screenPixelRadius = projectedRadii.x * control.screenSize.x;

    // bail out if there's nothing to march
	if (screenPixelRadius < 1.0f)
	{
        outColour = vec4(1.0f);
        return;
    }

    float stepSizePixels = screenPixelRadius / (control.maxStepsPerRay + 1.0f);

    float totalOcclusion = 0.0f;

    vec4 rand = texture(hbaoNoise, uv * control.noiseScale).rgba;
    const float alpha = 2.0f * M_PI / control.numRays;

    for(int dirIndex = 0; dirIndex < control.numRays; dirIndex++) {
        float angle = alpha * float(dirIndex);

        vec2 direction = RotateDirection(vec2(cos(angle), sin(angle)), rand.xy);
        vec2 rayPixels = (rand.zw * projectedRadii + 1.0f);

        for(int stepIndex = 0; stepIndex < control.maxStepsPerRay; stepIndex++) {
            vec2 SnappedUV = round(rayPixels * direction) * vec2(1.0f / control.screenSize.x, 1.0f / control.screenSize.y) + uv;
            vec3 sampleViewPos = ViewSpacePosAtScreenUV(SnappedUV);

            rayPixels += stepSizePixels;

            totalOcclusion += ComputeAO(viewPosition, viewNormal, sampleViewPos);
        }
    }
    float weight = 1.0f / (control.numRays * control.maxStepsPerRay);

    float ao = clamp(1.0f - totalOcclusion * weight * 2.0f, 0.0f, 1.0f);
    outColour = vec4(ao, ao, ao, 1.0f);
}