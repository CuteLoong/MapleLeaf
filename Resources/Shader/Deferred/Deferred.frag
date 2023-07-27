#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

layout(binding = 0) uniform UniformScene {
	mat4 view;
	vec3 cameraPosition;

	int lightsCount;
} scene;

struct Light {
	vec4 color;
	vec3 position;
	vec3 attenuation;
};

layout(binding = 1) buffer BufferLights {
	Light lights[];
} bufferLights;

layout(input_attachment_index = 0, binding = 2) uniform subpassInput inPosition;
layout(input_attachment_index = 1, binding = 3) uniform subpassInput inDiffuse;
layout(input_attachment_index = 2, binding = 4) uniform subpassInput inNormal;
layout(input_attachment_index = 3, binding = 5) uniform subpassInput inMaterial;


layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColour;

#include "Lighting.glsl"

void main() {
	vec3 worldPosition = subpassLoad(inPosition).rgb;
	vec4 screenPosition = scene.view * vec4(worldPosition, 1.0f);

	vec3 diffuse = subpassLoad(inDiffuse).rgb;
	vec3 normal = subpassLoad(inNormal).rgb;
	vec3 material = subpassLoad(inMaterial).rgb;

	float metallic = material.r;
	float roughness = material.g;
	
	vec3 N = normalize(normal);
	vec3 V = normalize(scene.cameraPosition - worldPosition);
	vec3 R = reflect(-V, N); 

	vec3 Lo = vec3(0.0f);

	for(int i = 0; i < scene.lightsCount; i++)
	{
		Light light = bufferLights.lights[i];
		vec3 L = light.position - worldPosition;
		float d = length(L);
		Lo += calcAttenuation(d, light.attenuation) * light.color.rgb;
	}
	
	outColour = vec4(diffuse, 1.0f);
}