#ifndef MISC_Utils_GLSL
#define MISC_Utils_GLSL

float Linear01Depth(float z) {
    z = z * 0.5 + 0.5; // NDC to [0, 1]
    return 1.0 / (camera.zBufferParams.x * z + camera.zBufferParams.y);
}

float LinearEyeDepth(float z) {
    z = z * 0.5 + 0.5; // NDC to [0, 1]
    return 1.0 / (camera.zBufferParams.z * z + camera.zBufferParams.w);
}

vec2 ScreenUVToStereoUV(vec2 uv, int viewIndex)
{
    int pixelOffset = int(camera.stereoPixelSize.x * viewIndex);
    float rawX = uv.x * camera.pixelSize.x;
    float jitterX = rawX - int(rawX);

    int pixelX = int(rawX) - pixelOffset;
    vec2 stereoUV = vec2((pixelX + jitterX) * camera.stereoPixelSize.z, uv.y);
    return stereoUV;
}

vec2 StereoUVToScreenUV(vec2 stereoUV, int viewIndex)
{
    stereoUV.x = clamp(stereoUV.x, 0.0f, 1.0f);
    int pixelOffset = int(camera.stereoPixelSize.x * viewIndex);
    float rawX = stereoUV.x * camera.stereoPixelSize.x;
    float jitterX = int(rawX) == 0 ? 0.5f : rawX - int(rawX);

    int pixelX = int(rawX) + pixelOffset;
    vec2 uv = vec2((pixelX + jitterX) * camera.pixelSize.z, stereoUV.y);
    return uv;
}

// inspired by keijiro's depth inverse projection
// https://github.com/keijiro/DepthInverseProjection
// constructs view space ray at the far clip plane from the screen uv
// then multiplies that ray by the linear 01 depth
vec3 ViewSpacePosAtScreenUV(vec2 uv)
{
    vec3 viewSpaceRay = vec3(camera.invProjection * (vec4(uv.x * 2.0f - 1.0f, (uv.y * 2.0f - 1.0f) * camera.projectionParams.x, -1.0f, 1.0f) * camera.projectionParams.z));
    float rawDepth = texture(inDepth, uv).r;
    return viewSpaceRay * Linear01Depth(rawDepth);
}

vec3 StereoViewSpacePosAtScreenUV(vec2 uv)
{
    int viewIndex = uv.x > 0.5f ? 1 : 0;
    vec2 stereoUV = ScreenUVToStereoUV(uv, viewIndex);
    vec3 viewSpaceRay = vec3(camera.invStereoProjection[viewIndex] * (vec4(stereoUV.x * 2.0f - 1.0f, (stereoUV.y * 2.0f - 1.0f) * camera.projectionParams.x, -1.0f, 1.0f) * camera.projectionParams.z));

    float rawDepth = texture(inDepth, uv).r;
    return viewSpaceRay * Linear01Depth(rawDepth);
}

vec3 StereoWorldSpacePosAtScreenUV(vec2 uv) 
{
    int viewIndex = uv.x > 0.5f ? 1 : 0;
    vec3 viewSpacePos = StereoViewSpacePosAtScreenUV(uv);
    return (camera.invStereoView[viewIndex] * vec4(viewSpacePos, 1.0f)).xyz;
}

vec3 StereoViewSpacePosAtStereoUV(vec2 uv, int viewIndex)
{
    uv.x = clamp(uv.x, 0.0f, 1.0f);
    vec3 viewSpaceRay = vec3(camera.invStereoProjection[viewIndex] * (vec4(uv.x * 2.0f - 1.0f, (uv.y * 2.0f - 1.0f) * camera.projectionParams.x, -1.0f, 1.0f) * camera.projectionParams.z));

    vec2 screenUV = StereoUVToScreenUV(uv, viewIndex); //  uv is in [0, 1] range, so we need to offset it by half the view index

    float rawDepth = texture(inDepth, screenUV).r;
    return viewSpaceRay * Linear01Depth(rawDepth);
}

vec3 ViewNormalAtScreenUV(vec2 uv)
{
    // get current pixel's view space position
    vec3 viewSpacePos_c = ViewSpacePosAtScreenUV(uv + vec2(0.0f, 0.0f));

    // get view space position at 1 pixel offsets in each major direction
    vec3 viewSpacePos_r = ViewSpacePosAtScreenUV(uv + vec2(camera.pixelSize.z, 0.0f));
    vec3 viewSpacePos_d = ViewSpacePosAtScreenUV(uv + vec2(0.0f, camera.pixelSize.w));

    // get the difference between the current and each offset position
    vec3 hDeriv = viewSpacePos_r - viewSpacePos_c;
    vec3 vDeriv = viewSpacePos_d - viewSpacePos_c;

    // get view space normal from the cross product of the diffs
    vec3 viewNormal = normalize(cross(vDeriv, hDeriv));

    return viewNormal;
}

vec3 StereoViewNormalAtScreenUV(vec2 uv, int viewIndex)
{
    // get current pixel's view space position
    vec3 viewSpacePos_c = StereoViewSpacePosAtStereoUV(uv + vec2(0.0f, 0.0f), viewIndex);

    // get view space position at 1 pixel offsets in each major direction
    vec3 viewSpacePos_r = StereoViewSpacePosAtStereoUV(uv + vec2(camera.stereoPixelSize.z, 0.0f), viewIndex);
    vec3 viewSpacePos_d = StereoViewSpacePosAtStereoUV(uv + vec2(0.0f, camera.stereoPixelSize.w), viewIndex);

    // get the difference between the current and each offset position
    vec3 hDeriv = viewSpacePos_r - viewSpacePos_c;
    vec3 vDeriv = viewSpacePos_d - viewSpacePos_c;

    // get view space normal from the cross product of the diffs
    vec3 viewNormal = normalize(cross(vDeriv, hDeriv));

    return viewNormal;
}

vec3 StereoViewNormalAtStereoUVImproved(vec2 uv, int viewIndex)
{
    // get current pixel's view space position
    vec3 viewSpacePos_c = StereoViewSpacePosAtStereoUV(uv + vec2(0.0f, 0.0f), viewIndex);

    // get view space position at 1 pixel offsets in each major direction
    vec3 viewSpacePos_l = StereoViewSpacePosAtStereoUV(uv - vec2(camera.stereoPixelSize.z, 0.0f), viewIndex);
    vec3 viewSpacePos_r = StereoViewSpacePosAtStereoUV(uv + vec2(camera.stereoPixelSize.z, 0.0f), viewIndex);
    vec3 viewSpacePos_u = StereoViewSpacePosAtStereoUV(uv - vec2(0.0f, camera.stereoPixelSize.w), viewIndex);
    vec3 viewSpacePos_d = StereoViewSpacePosAtStereoUV(uv + vec2(0.0f, camera.stereoPixelSize.w), viewIndex);

    // get the difference between the current and each offset position
    vec3 l = viewSpacePos_c - viewSpacePos_l;
    vec3 r = viewSpacePos_r - viewSpacePos_c;
    vec3 d = viewSpacePos_c - viewSpacePos_d;
    vec3 u = viewSpacePos_u - viewSpacePos_c;

    vec3 hDeriv = abs(l.z) < abs(r.z) ? l : r;
    vec3 vDeriv = abs(d.z) < abs(u.z) ? d : u;

    // get view space normal from the cross product of the diffs
    vec3 viewNormal = normalize(cross(hDeriv, vDeriv));

    return viewNormal;
}

#endif