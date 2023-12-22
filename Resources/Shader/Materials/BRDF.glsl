#ifndef BRDF_GLSL
#define BRDF_GLSL

#include <Misc/Constants.glsl>
#include "Fresnel.glsl"
#include "Microfacet.glsl"

/** Disney's diffuse reflection.
    Based on https://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
*/
vec3 DiffuseReflectionDisney(vec3 diffuseColor, float roughness, vec3 N, vec3 L, vec3 V)
{
    vec3 H = normalize(L + V);
    float VoH = clamp(dot(V, H), 0.0f, 1.0f);
    float LoH = clamp(dot(L, H), 0.0f, 1.0f);

    float VoN = clamp(dot(V, N), 0.0f, 1.0f);
    float NoL = clamp(dot(N, L), 0.0f, 1.0f);

    if(min(VoN, NoL) < 0.001f) return vec3(0.0f);

    float Fd90 = 0.5f + 2.0f * LoH * LoH * roughness;
    float Fd0 = 1.0f;

    float wiScatter = evalFresnelSchlick(Fd0, Fd90, LoH);
    float woScatter = evalFresnelSchlick(Fd0, Fd90, VoH);

    return diffuseColor * wiScatter * woScatter /* * INV_M_PI */;
}

float evalPdfDiffuseReflectionDisney(vec3 V, vec3 N, vec3 L) {
    float NoV = clamp(dot(N, V), 0.0f, 1.0f);
    float NoL = clamp(dot(N, L), 0.0f, 1.0f);

    if(min(NoV, NoL) < 0.001f) return 0.0f;

    return NoL * INV_M_PI;
}

/** Specular reflection using microfacets.
*/
vec3 SpecularReflectionMicrofacet(vec3 specularColor, float roughness,vec3 N, vec3 L, vec3 V)
{
    vec3 H = normalize(L + V);
    
    float VoH = clamp(dot(V, H), 0.0f, 1.0f);
    float NoH = clamp(dot(N, H), 0.0f, 1.0f);
    float LoH = clamp(dot(L, H), 0.0f, 1.0f);

    float NoV = clamp(dot(N, V), 0.0f, 1.0f);
    float NoL = clamp(dot(N, L), 0.0f, 1.0f);

    if(min(NoV, NoL) < 0.001f) return vec3(0.0f);

    float alpha = roughness * roughness;

    float D = evalNdfGGX(alpha, NoH);
    float G = evalMaskingSmithGGXCorrelated(alpha, NoV, NoL);
    vec3 F = evalFresnelSchlick(specularColor, vec3(1.0f), VoH);

    return D * G * F * 0.25f / (/* NoL * */ NoV);
}

float evalPdfReflectionMicrofacet(vec3 V, vec3 N, vec3 L, float roughness) {
    vec3 H = normalize(L + V);
    float NoH = clamp(dot(N, H), 0.0f, 1.0f);
    float NoV = clamp(dot(N, V), 0.0f, 1.0f);
    float NoL = clamp(dot(N, L), 0.0f, 1.0f);
    float VoH = clamp(dot(V, H), 0.0f, 1.0f);

    if(min(NoV, NoL) < 0.001f) return 0.0f;

    float alpha = roughness * roughness;

    return evalPdfGGX_NDF(alpha, NoH) / (4.0f * VoH);
}

#endif // BRDF_GLSL