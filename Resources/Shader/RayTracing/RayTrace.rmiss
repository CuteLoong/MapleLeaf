#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

layout(set = 0, binding = 9) uniform samplerCube SkyboxCubeMap;

#include <Misc/RayTracingCommon.glsl>

layout(location = 0) rayPayloadInEXT HitPayLoad prd;

void main()
{
    vec3 rayDirection = gl_WorldRayDirectionEXT;
    vec3 cubemapColour = texture(SkyboxCubeMap, normalize(rayDirection)).rgb;
    // prd.Lo = vec3(cubemapColour);
    // prd.Lo = vec3(cubemapColour) * prd.accBRDF / prd.accPDF;
	// prd.done = 1;
    prd.Lo = vec3(0.0, 0.0, 0.0);
}