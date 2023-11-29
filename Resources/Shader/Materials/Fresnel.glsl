#ifndef FRESNEL_GLSL
#define FRESNEL_GLSL

/** Evaluates the Fresnel term using Schlick's approximation.
    Introduced in http://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf

    The Fresnel term equals f0 at normal incidence, and approaches f90=1.0 at 90 degrees.
    The formulation below is generalized to allow both f0 and f90 to be specified.

    \param[in] f0 Specular reflectance at normal incidence (0 degrees).
    \param[in] f90 Reflectance at orthogonal incidence (90 degrees), which should be 1.0 for specular surface reflection.
    \param[in] cosTheta Cosine of angle between microfacet normal and incident direction (LdotH).
    \return Fresnel term.
*/
vec3 evalFresnelSchlick(vec3 f0, vec3 f90, float cosTheta)
{
    return f0 + (f90 - f0) * pow(max(1 - cosTheta, 0), 5); // Clamp to avoid NaN if cosTheta = 1+epsilon
}

float evalFresnelSchlick(float f0, float f90, float cosTheta)
{
    return f0 + (f90 - f0) * pow(max(1 - cosTheta, 0), 5); // Clamp to avoid NaN if cosTheta = 1+epsilon
}

/** Disney's diffuse term. Based on https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
*/
float disneyDiffuseFresnel(float NdotV, float NdotL, float LdotH, float roughness)
{
    float fd90 = 0.5 + 2 * LdotH * LdotH * roughness;
    float fd0 = 1;
    float lightScatter = evalFresnelSchlick(fd0, fd90, NdotL).r;
    float viewScatter = evalFresnelSchlick(fd0, fd90, NdotV).r;
    return lightScatter * viewScatter;
}

#endif