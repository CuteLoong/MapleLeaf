const float PI = 3.1415926535897932384626433832795;

float calcAttenuation(float distance, vec3 attenuation)
{
    return 1.0f / (attenuation.x + attenuation.y * distance + attenuation.z * distance * distance);
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