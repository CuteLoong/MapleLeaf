#ifndef MICROFACET_GLSL
#define MICROFACET_GLSL

#include <Misc/Constants.glsl>

/** Evaluates the GGX (Trowbridge-Reitz) normal distribution function (D).

    Introduced by Trowbridge and Reitz, "Average irregularity representation of a rough surface for ray reflection", Journal of the Optical Society of America, vol. 65(5), 1975.
    See the correct normalization factor in Walter et al. https://dl.acm.org/citation.cfm?id=2383874
    We use the simpler, but equivalent expression in Eqn 19 from http://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_notes.pdf

    For microfacet models, D is evaluated for the direction h to find the density of potentially active microfacets (those for which microfacet normal m = h).
    The 'alpha' parameter is the standard GGX width, e.g., it is the square of the linear roughness parameter in Disney's BRDF.
    Note there is a singularity (0/0 = NaN) at NdotH = 1 and alpha = 0, so alpha should be clamped to some epsilon.

    \param[in] alpha GGX width parameter (should be clamped to small epsilon beforehand).
    \param[in] cosTheta Dot product between shading normal and half vector, in positive hemisphere.
    \return D(h)
*/
float evalNdfGGX(float alpha, float cosTheta)
{
    float a2 = alpha * alpha;
    float d = ((cosTheta * a2 - cosTheta) * cosTheta + 1);
    return a2 / (d * d * M_PI);
}

/** Evaluates the PDF for sampling the GGX normal distribution function using Walter et al. 2007's method.
    See https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf

    \param[in] alpha GGX width parameter (should be clamped to small epsilon beforehand).
    \param[in] cosTheta Dot product between shading normal and half vector, in positive hemisphere.
    \return D(h) * cosTheta
*/
float evalPdfGGX_NDF(float alpha, float cosTheta)
{
    return evalNdfGGX(alpha, cosTheta) * cosTheta;
}

/** Samples the GGX (Trowbridge-Reitz) normal distribution function (D) using Walter et al. 2007's method.
    Note that the sampled half vector may lie in the negative hemisphere. Such samples should be discarded.
    See Eqn 35 & 36 in https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
    See Listing A.1 in https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf

    \param[in] alpha GGX width parameter (should be clamped to small epsilon beforehand).
    \param[in] u Uniform random number (2D).
    \param[out] pdf Sampling probability.
    \return Sampled half vector in local space.
*/
vec3 sampleGGX_NDF(float alpha, vec2 u, out float pdf)
{
    float alphaSqr = alpha * alpha;
    float phi = u.x * M_2PI;
    float tanThetaSqr = alphaSqr * u.y / (1 - u.y);
    float cosTheta = 1 / sqrt(1 + tanThetaSqr);
    float r = sqrt(max(1 - cosTheta * cosTheta, 0));

    pdf = evalPdfGGX_NDF(alpha, cosTheta);
    return vec3(cos(phi) * r, sin(phi) * r, cosTheta);
}

/** Evaluates the Smith masking function (G1) for the GGX normal distribution.
    See Eq 34 in https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf

    The evaluated direction is assumed to be in the positive hemisphere relative the half vector.
    This is the case when both incident and outgoing direction are in the same hemisphere, but care should be taken with transmission.

    \param[in] alphaSqr Squared GGX width parameter.
    \param[in] cosTheta Dot product between shading normal and evaluated direction, in the positive hemisphere.
*/
float evalG1GGX(float alphaSqr, float cosTheta)
{
    if (cosTheta <= 0) return 0;
    float cosThetaSqr = cosTheta * cosTheta;
    float tanThetaSqr = max(1 - cosThetaSqr, 0) / cosThetaSqr;
    return 2 / (1 + sqrt(1 + alphaSqr * tanThetaSqr));
}

/** Evaluates the PDF for sampling the GGX distribution of visible normals (VNDF).
    See http://jcgt.org/published/0007/04/01/paper.pdf

    \param[in] alpha GGX width parameter (should be clamped to small epsilon beforehand).
    \param[in] wi Incident direction in local space, in the positive hemisphere.
    \param[in] h Half vector in local space, in the positive hemisphere.
    \return D_V(h) = G1(wi) * D(h) * max(0,dot(wi,h)) / wi.z
*/
float evalPdfGGX_VNDF(float alpha, vec3 wi, vec3 h)
{
    float G1 = evalG1GGX(alpha * alpha, wi.z);
    float D = evalNdfGGX(alpha, h.z);
    return G1 * D * max(0.f, dot(wi, h)) / wi.z;
}

/** Samples the GGX (Trowbridge-Reitz) using the distribution of visible normals (VNDF).
    The GGX VDNF yields significant variance reduction compared to sampling of the GGX NDF.
    See http://jcgt.org/published/0007/04/01/paper.pdf

    \param[in] alpha Isotropic GGX width parameter (should be clamped to small epsilon beforehand).
    \param[in] wi Incident direction in local space, in the positive hemisphere.
    \param[in] u Uniform random number (2D).
    \param[out] pdf Sampling probability.
    \return Sampled half vector in local space, in the positive hemisphere.
*/
vec3 sampleGGX_VNDF(float alpha, vec3 wi, vec2 u, out float pdf)
{
    float alpha_x = alpha, alpha_y = alpha;

    // Transform the view vector to the hemisphere configuration.
    vec3 Vh = normalize(vec3(alpha_x * wi.x, alpha_y * wi.y, wi.z));

    // Construct orthonormal basis (Vh,T1,T2).
    vec3 T1 = (Vh.z < 0.9999f) ? normalize(cross(vec3(0, 0, 1), Vh)) : vec3(1, 0, 0); // TODO: fp32 precision
    vec3 T2 = cross(Vh, T1);

    // Parameterization of the projected area of the hemisphere.
    float r = sqrt(u.x);
    float phi = (2.f * M_PI) * u.y;
    float t1 = r * cos(phi);
    float t2 = r * sin(phi);
    float s = 0.5f * (1.f + Vh.z);
    t2 = (1.f - s) * sqrt(1.f - t1 * t1) + s * t2;

    // Reproject onto hemisphere.
    vec3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.f, 1.f - t1 * t1 - t2 * t2)) * Vh;

    // Transform the normal back to the ellipsoid configuration. This is our half vector.
    vec3 h = normalize(vec3(alpha_x * Nh.x, alpha_y * Nh.y, max(0.f, Nh.z)));

    pdf = evalPdfGGX_VNDF(alpha, wi, h);
    return h;
}

/** Evaluates the Smith lambda function for the GGX normal distribution.
    See Eq 72 in http://jcgt.org/published/0003/02/03/paper.pdf

    \param[in] alphaSqr Squared GGX width parameter.
    \param[in] cosTheta Dot product between shading normal and the evaluated direction, in the positive hemisphere.
*/
float evalLambdaGGX(float alphaSqr, float cosTheta)
{
    if (cosTheta <= 0) return 0;
    float cosThetaSqr = cosTheta * cosTheta;
    float tanThetaSqr = max(1 - cosThetaSqr, 0) / cosThetaSqr;
    return 0.5 * (-1 + sqrt(1 + alphaSqr * tanThetaSqr));
}

/** Evaluates the separable form of the masking-shadowing function for the GGX normal distribution, using Smith's approximation.
    See Eq 98 in http://jcgt.org/published/0003/02/03/paper.pdf

    \param[in] alpha GGX width parameter (should be clamped to small epsilon beforehand).
    \param[in] cosThetaI Dot product between shading normal and incident direction, in positive hemisphere.
    \param[in] cosThetaO Dot product between shading normal and outgoing direction, in positive hemisphere.
    \return G(cosThetaI, cosThetaO)
*/
float evalMaskingSmithGGXSeparable(float alpha, float cosThetaI, float cosThetaO)
{
    float alphaSqr = alpha * alpha;
    float lambdaI = evalLambdaGGX(alphaSqr, cosThetaI);
    float lambdaO = evalLambdaGGX(alphaSqr, cosThetaO);
    return 1 / ((1 + lambdaI) * (1 + lambdaO));
}

/** Evaluates the height-correlated form of the masking-shadowing function for the GGX normal distribution, using Smith's approximation.
    See Eq 99 in http://jcgt.org/published/0003/02/03/paper.pdf

    Eric Heitz recommends using it in favor of the separable form as it is more accurate and of similar complexity.
    The function is only valid for cosThetaI > 0 and cosThetaO > 0  and should be clamped to 0 otherwise.

    \param[in] alpha GGX width parameter (should be clamped to small epsilon beforehand).
    \param[in] cosThetaI Dot product between shading normal and incident direction, in positive hemisphere.
    \param[in] cosThetaO Dot product between shading normal and outgoing direction, in positive hemisphere.
    \return G(cosThetaI, cosThetaO)
*/
float evalMaskingSmithGGXCorrelated(float alpha, float cosThetaI, float cosThetaO)
{
    float alphaSqr = alpha * alpha;
    float lambdaI = evalLambdaGGX(alphaSqr, cosThetaI);
    float lambdaO = evalLambdaGGX(alphaSqr, cosThetaO);
    return 1 / (1 + lambdaI + lambdaO);
}

#endif // MICROFACET_GLSL