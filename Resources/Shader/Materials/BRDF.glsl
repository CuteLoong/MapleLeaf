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

    float Fd90 = 0.5 + 2.0 * VoH * VoH * roughness;
    float Fd0 = 1.0f;

    float wiScatter = evalFresnelSchlick(Fd0, Fd90, LoH);
    float woScatter = evalFresnelSchlick(Fd0, Fd90, VoH);

    return diffuseColor * wiScatter * woScatter * INV_M_PI;
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

    float D = evalNdfGGX(roughness, NoH);
    float G = evalMaskingSmithGGXCorrelated(roughness, NoL, NoV);
    vec3 F = evalFresnelSchlick(specularColor, vec3(1.0f), LoH);

    return D * G * F * 0.25f / (NoL * NoV);
}

#endif // BRDF_GLSL