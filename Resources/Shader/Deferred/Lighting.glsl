const float M_PI = 3.1415926535897932384626433832795;

float calcAttenuation(float distance, vec3 attenuation)
{
    return 1.0f / (attenuation.x + attenuation.y * distance + attenuation.z * distance * distance);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) 
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = M_PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;

    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float shadowFactor(vec4 shadowCoords) 
{
    vec3 ndc = shadowCoords.xyz / shadowCoords.w;
    ndc.xyz = ndc.xyz * 0.5 + 0.5;
    ndc.y = 1.0 - ndc.y;

    float bias = 0.005;

    if (abs(ndc.x) > 1.0f || abs(ndc.y) > 1.0f || abs(ndc.z) > 1.0f) 
    {
		return 0.0f;
	}
	
	float shadowValue = texture(inShadowMap, ndc.xy).r;

	if (ndc.z - bias > shadowValue) 
    {
		return 0.0f;
	}

    return shadowValue;
}