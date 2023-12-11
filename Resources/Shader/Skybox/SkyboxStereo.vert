#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

layout(set = 0, binding = 1) uniform UniformSkybox {
	mat4 transform;
	vec4 baseColour;
	float blendFactor;
} uniformSkybox;

#include <Misc/Camera.glsl>

const vec3 positions[36] = vec3[](
    vec3(-1.0f, -1.0f, -1.0f),  
    vec3( 1.0f, -1.0f, -1.0f),  
    vec3( 1.0f,  1.0f, -1.0f),  
    vec3( 1.0f,  1.0f, -1.0f),  
    vec3(-1.0f,  1.0f, -1.0f),  
    vec3(-1.0f, -1.0f, -1.0f),  

    vec3(-1.0f, -1.0f,  1.0f),  
    vec3( 1.0f, -1.0f,  1.0f),  
    vec3( 1.0f,  1.0f,  1.0f),  
    vec3( 1.0f,  1.0f,  1.0f),  
    vec3(-1.0f,  1.0f,  1.0f),  
    vec3(-1.0f, -1.0f,  1.0f),  

    vec3(-1.0f,  1.0f,  1.0f),  
    vec3(-1.0f,  1.0f, -1.0f),  
    vec3(-1.0f, -1.0f, -1.0f),  
    vec3(-1.0f, -1.0f, -1.0f),  
    vec3(-1.0f, -1.0f,  1.0f),  
    vec3(-1.0f,  1.0f,  1.0f),  

    vec3( 1.0f,  1.0f,  1.0f),  
    vec3( 1.0f,  1.0f, -1.0f),  
    vec3( 1.0f, -1.0f, -1.0f),  
    vec3( 1.0f, -1.0f, -1.0f),  
    vec3( 1.0f, -1.0f,  1.0f),  
    vec3( 1.0f,  1.0f,  1.0f),  

    vec3(-1.0f, -1.0f, -1.0f),  
    vec3( 1.0f, -1.0f, -1.0f),  
    vec3( 1.0f, -1.0f,  1.0f),  
    vec3( 1.0f, -1.0f,  1.0f),  
    vec3(-1.0f, -1.0f,  1.0f),  
    vec3(-1.0f, -1.0f, -1.0f),  

    vec3(-1.0f,  1.0f, -1.0f),  
    vec3( 1.0f,  1.0f, -1.0f),  
    vec3( 1.0f,  1.0f,  1.0f),  
    vec3( 1.0f,  1.0f,  1.0f),  
    vec3(-1.0f,  1.0f,  1.0f),  
    vec3(-1.0f,  1.0f, -1.0f)  
    );

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outUVW;

void main() {
	vec4 worldPosition = vec4(positions[gl_VertexIndex], 1.0f);
	
	gl_Position = worldPosition;

	outPosition = worldPosition.xyz;
    outUVW = positions[gl_VertexIndex];
}