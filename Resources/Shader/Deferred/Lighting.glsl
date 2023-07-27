const float PI = 3.1415926535897932384626433832795;

float calcAttenuation(float distance, vec3 attenuation)
{
    return 1.0f / (attenuation.x + attenuation.y * distance + attenuation.z * distance * distance);
}