#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

layout(set=0, binding = 0) uniform UniformScene {
	mat4 view;
	mat4 shadowMatrix;
	vec3 cameraPosition;

	int pointLightsCount;
	int directionalLightsCount;
} scene;

struct PointLight {
	vec4 color;
	vec3 position;
	vec3 attenuation;
};

struct DirectionalLight {
	vec4 color;
	vec3 direction;
};

layout(set=0, binding = 1) buffer BufferPointLights {
	PointLight lights[];
} bufferPointLights;

layout(set=0, binding = 2) buffer BufferDirectionalLights {
	DirectionalLight lights[];
} bufferDirectionalLights;

layout(set=0, binding = 3) uniform sampler2D inPosition;
layout(set=0, binding = 4) uniform sampler2D inDiffuse;
layout(set=0, binding = 5) uniform sampler2D inNormal;
layout(set=0, binding = 6) uniform sampler2D inMaterial;
layout(set=0, binding = 7) uniform sampler2D inShadowMap;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColour;

#include "Lighting.glsl"

void main() {
	vec2 uv = vec2(inUV.x, 1.0f - inUV.y);

	vec3 worldPosition = texture(inPosition, uv).rgb;
	vec4 screenPosition = scene.view * vec4(worldPosition, 1.0f);
	vec4 shadowCoords = scene.shadowMatrix * vec4(worldPosition, 1.0f);

	vec3 diffuse = texture(inDiffuse, uv).rgb;
	vec3 normal = texture(inNormal, uv).rgb;
	vec3 material = texture(inMaterial, uv).rgb;

	float metallic = material.r;
	float roughness = material.g;
	
	vec3 N = normalize(normal);
	vec3 V = normalize(scene.cameraPosition - worldPosition);
	vec3 R = reflect(-V, N); 

	vec3 Lo = vec3(0.0f);

	for(int i = 1; i <= scene.pointLightsCount; i++)
	{
		PointLight light = bufferPointLights.lights[i];
		vec3 L = light.position - worldPosition;
		float d = length(L);
		Lo += calcAttenuation(d, light.attenuation) * light.color.rgb;
	}

	for(int i = 1; i <= scene.directionalLightsCount; i++)
	{
		DirectionalLight light = bufferDirectionalLights.lights[i];
		Lo += light.color.rgb;
	}

	float shadowValue = shadowFactor(shadowCoords);
	
	outColour = vec4(diffuse * Lo * shadowValue, 1.0f) ;
}