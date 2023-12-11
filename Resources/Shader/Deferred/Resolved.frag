#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 inUV;

layout(set=0, binding=1) uniform sampler2D inDepth;
layout(set=0, binding=2) uniform sampler2D inNormal;
layout(set=0, binding=3) uniform sampler2D inMaterial;
layout(set=0, binding = 4) uniform sampler2D inDiffuse;
layout(set=0, binding = 5) uniform sampler2D LightingMap;
layout(set=0, binding = 6) uniform sampler2D ReflectionColorMap;
layout(set=0, binding = 7) uniform sampler2D PreIntegratedBRDF;

#include <Misc/Camera.glsl>
#include <Misc/Utils.glsl>
#include <Misc/Constants.glsl>

layout(location = 0) out vec4 outColor;

vec3 LookUpPreIntegratedBRDF(float NoV, float roughness)
{
    return texture(PreIntegratedBRDF, vec2(roughness, NoV)).xyz;
}

void main()
{
	vec2 uv = vec2(inUV.x, 1.0f - inUV.y);
    int viewIndex = inUV.x < 0.5f ? 0 : 1;

    vec3 normalWS = texture(inNormal, uv).xyz;
    vec4 ssrColor = texture(ReflectionColorMap, uv);
    float roughness = texture(inMaterial, uv).g;
    float metallic = texture(inMaterial, uv).r;
    vec3 diffuseColor = texture(inDiffuse, uv).xyz;
    vec3 F0 = vec3(0.04f);
    F0 = mix(F0, diffuseColor, metallic);

    vec3 originWS = StereoWorldSpacePosAtScreenUV(uv);
    vec3 eyeWS = camera.cameraStereoPosition[viewIndex].xyz;
    vec3 rayWS = normalize(originWS - eyeWS);

    float NoV = clamp(dot(normalWS, -rayWS), 0.0f, 1.0f);
    vec3 brdf = LookUpPreIntegratedBRDF(NoV, roughness);

    vec3 dfg = F0 * brdf.x + brdf.y;

    vec3 reflectionColor = ssrColor.xyz * dfg * ssrColor.w;

    vec3 lighting = texture(LightingMap, uv).xyz;

    outColor = vec4(lighting + reflectionColor, 1.0f);
}