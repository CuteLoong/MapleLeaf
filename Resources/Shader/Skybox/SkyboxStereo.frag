#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 1) uniform UniformSkybox {
	mat4 transform;
	vec4 baseColour;
	float blendFactor;
} uniformSkybox;

layout(set = 0, binding = 2) uniform samplerCube SkyboxCubeMap;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inUVW;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outDiffuse;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outMaterial;

void main() {
	vec3 cubemapColour = texture(SkyboxCubeMap, inUVW).rgb;
	vec3 colour = mix(uniformSkybox.baseColour.rgb, cubemapColour, uniformSkybox.blendFactor);
	
	outPosition = vec4(inPosition, 1.0f);
	outDiffuse = vec4(colour, 1.0f);
	outNormal = vec4(0.0f);
	outMaterial = vec4(0.0f);
}