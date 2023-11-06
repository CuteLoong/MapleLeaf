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

vec3 StereoViewSpacePosAtScreenUV(vec2 uv, int viewIndex)
{
    vec3 viewSpaceRay = vec3(camera.invStereoProjection[viewIndex] * (vec4(uv.x * 2.0f - 1.0f, (uv.y * 2.0f - 1.0f) * camera.projectionParams.x, -1.0f, 1.0f) * camera.projectionParams.z));

    float viewOffset = float(viewIndex) * 0.5f;
    float uvx = clamp(uv.x / 2.0f + viewOffset, viewOffset + camera.pixelSize.z, 0.5f + viewOffset - camera.pixelSize.z); // clamp to avoid sampling from the other view, and avoid sampling by linear interpolation
    vec2 screenUV = vec2(uvx, uv.y); // uv is in [0, 1] range, so we need to offset it by half the view index

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
    vec3 viewSpacePos_c = StereoViewSpacePosAtScreenUV(uv + vec2(0.0f, 0.0f), viewIndex);

    // get view space position at 1 pixel offsets in each major direction
    vec3 viewSpacePos_r = StereoViewSpacePosAtScreenUV(uv + vec2(camera.pixelSize.z, 0.0f), viewIndex);
    vec3 viewSpacePos_d = StereoViewSpacePosAtScreenUV(uv + vec2(0.0f, camera.pixelSize.w), viewIndex);

    // get the difference between the current and each offset position
    vec3 hDeriv = viewSpacePos_r - viewSpacePos_c;
    vec3 vDeriv = viewSpacePos_d - viewSpacePos_c;

    // get view space normal from the cross product of the diffs
    vec3 viewNormal = normalize(cross(vDeriv, hDeriv));

    return viewNormal;
}

vec3 StereoViewNormalAtScreenUVImproved(vec2 uv, int viewIndex)
{
    // get current pixel's view space position
    vec3 viewSpacePos_c = StereoViewSpacePosAtScreenUV(uv + vec2(0.0f, 0.0f), viewIndex);

    // get view space position at 1 pixel offsets in each major direction
    vec3 viewSpacePos_l = StereoViewSpacePosAtScreenUV(uv - vec2(camera.pixelSize.z, 0.0f), viewIndex);
    vec3 viewSpacePos_r = StereoViewSpacePosAtScreenUV(uv + vec2(camera.pixelSize.z, 0.0f), viewIndex);
    vec3 viewSpacePos_u = StereoViewSpacePosAtScreenUV(uv - vec2(0.0f, camera.pixelSize.w), viewIndex);
    vec3 viewSpacePos_d = StereoViewSpacePosAtScreenUV(uv + vec2(0.0f, camera.pixelSize.w), viewIndex);

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