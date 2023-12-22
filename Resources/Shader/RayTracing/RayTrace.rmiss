#version 460
#extension GL_EXT_ray_tracing : require

layout(set = 0, binding = 9) uniform samplerCube SkyboxCubeMap;

layout(location = 0) rayPayloadInEXT vec3 hitValue;

void main()
{
    vec3 rayDirection = gl_WorldRayDirectionEXT;
    vec3 cubemapColour = texture(SkyboxCubeMap, normalize(rayDirection)).rgb;
    // hitValue = vec3(cubemapColour);
    hitValue = vec3(0.0, 0.0, 0.0);
}