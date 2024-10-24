#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

#include <Misc/Camera.glsl>

layout(set=0, binding = 1) uniform UniformScene {
	mat4 shadowMatrix;

	int pointLightsCount;
	int directionalLightsCount;
} scene;

struct PointLight {
	vec4 color;
	vec3 position;
	float pad;
	vec3 attenuation;
	float pad1;
};

struct DirectionalLight {
	vec4 color;
	vec3 direction;
	float pad;
};

layout(set=0, binding = 2) buffer BufferPointLights {
	PointLight lights[];
} bufferPointLights;

layout(set=0, binding = 3) buffer BufferDirectionalLights {
	DirectionalLight lights[];
} bufferDirectionalLights;

layout(set=0, binding = 4) uniform sampler2D inPosition;
layout(set=0, binding = 5) uniform sampler2D inDiffuse;
layout(set=0, binding = 6) uniform sampler2D inNormal;
layout(set=0, binding = 7) uniform sampler2D inMaterial;
layout(set=0, binding = 8) uniform sampler2D inShadowMap;
// layout(set=0, binding = 9) uniform sampler2D inAOMap;

layout(set=0, binding = 10) uniform sampler2D samplerBRDF;
layout(set=0, binding = 11) uniform samplerCube samplerIrradiance;
layout(set=0, binding = 12) uniform samplerCube samplerPrefiltered;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColour;

#include "Lighting.glsl"
#include <Misc/Constants.glsl>
#include <Materials/Fresnel.glsl>
#include <Materials/BRDF.glsl>

void main() {
	vec2 uv = vec2(inUV.x, 1.0f - inUV.y);

	vec3 worldPosition = texture(inPosition, uv).rgb;
	vec4 screenPosition = camera.view * vec4(worldPosition, 1.0f);
	vec4 shadowCoords = scene.shadowMatrix * vec4(worldPosition, 1.0f);

	vec3 diffuse = texture(inDiffuse, uv).rgb;
	vec3 normal = texture(inNormal, uv).rgb;
	vec3 material = texture(inMaterial, uv).rgb;
	// vec3 ao = texture(inAOMap, uv).rgb;

	float metallic = material.r;
	float roughness = material.g;
	
	vec3 Lo = vec3(0.0f);

	if(normal != vec3(0.0f)) {
		vec3 N = normalize(normal);
		vec3 V = normalize(camera.cameraPosition.xyz - worldPosition);
		vec3 R = reflect(-V, N);

		float NdotV = clamp(dot(N, V), 0.0f, 1.0f);

		vec3 F0 = vec3(0.04f);
		F0 = mix(F0, diffuse, metallic);
		for(int i = 1; i <= scene.pointLightsCount; i++)
		{
			PointLight light = bufferPointLights.lights[i];
			vec3 L = light.position - worldPosition;
			float d = length(L);
			L = normalize(L);
			vec3 radiance = calcAttenuation(d, light.attenuation) * light.color.rgb;

			vec3 brdf = (1.0f - metallic) * DiffuseReflectionDisney(diffuse, roughness, N, L, V) + SpecularReflectionMicrofacet(F0, roughness, N, L, V);

			// Lo += brdf * radiance;
		}

		for(int i = 1; i <= scene.directionalLightsCount; i++)
		{
			DirectionalLight light = bufferDirectionalLights.lights[i];
			vec3 L = normalize(-light.direction);

			float NoL = clamp(dot(N, L), 0.0f, 1.0f);
			vec3 radiance = light.color.rgb;

			vec3 brdf = (1 - metallic) * DiffuseReflectionDisney(diffuse, roughness, N, L, V) * NoL + SpecularReflectionMicrofacet(F0, roughness, N, L, V);

			float shadowValue = shadowFactor(shadowCoords);

			// Lo += brdf * radiance * shadowValue;
			// Lo += brdf * radiance;
		}

		vec3 brdfPreIntegrated = texture(samplerBRDF, vec2(NdotV, roughness)).rgb;
		vec3 reflection = prefilteredReflection(R, roughness, samplerPrefiltered).rgb;	
		vec3 specular = reflection * (F0 * brdfPreIntegrated.r + brdfPreIntegrated.g);

		vec3 irradiance = texture(samplerIrradiance, N).rgb;
		vec3 diffuseLo = irradiance * (1 - metallic) * diffuse * brdfPreIntegrated.b * INV_M_PI;

		vec3 ambient = (diffuseLo * 2.5f + specular * 0.2f); //  
		// vec3 ambient = (diffuseLo + specular) * 2.0f ;
		// vec3 ambient = (specular) * 1.0f ;

		Lo += ambient;
	}
	else {
		Lo = diffuse;
	}

	// vec3 ambient = vec3(0.1f) * diffuse * ao;
	// Lo += ambient;
	
	outColour = vec4(Lo, 1.0f);
}