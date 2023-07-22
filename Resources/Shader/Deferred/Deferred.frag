#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

layout(binding = 0) uniform UniformScene {
	mat4 view;
	vec3 cameraPosition;
} scene;

layout(binding = 1) uniform sampler2D samplerPosition;
layout(binding = 2) uniform sampler2D samplerDiffuse;
layout(binding = 3) uniform sampler2D samplerNormal;
layout(binding = 4) uniform sampler2D samplerMaterial;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColour;

void main() {
	vec2 flipUv = vec2(inUV.x, 1.0 - inUV.y);
	vec3 worldPosition = texture(samplerPosition, flipUv).rgb;
	vec4 screenPosition = scene.view * vec4(worldPosition, 1.0f);

	vec4 diffuse = texture(samplerDiffuse, flipUv);
	// vec3 normal = texture(samplerNormal, inUV).rgb;
	// vec3 material = texture(samplerMaterial, inUV).rgb;

	// float metallic = material.r;
	// float roughness = material.g;
	// bool ignoreFog = material.b == (1.0f / 3.0f) || material.b == (3.0f / 3.0f);
	// bool ignoreLighting = material.b == (2.0f / 3.0f) || material.b == (3.0f / 3.0f);
	
	// vec3 N = normalize(normal);
	// vec3 V = normalize(scene.cameraPosition - worldPosition);
	// vec3 R = reflect(-V, N); 

	
	outColour = vec4(diffuse.rgb, 1.0f);
}