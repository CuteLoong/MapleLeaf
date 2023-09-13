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

layout(set=0, input_attachment_index = 0, binding = 3) uniform subpassInput inPosition;
layout(set=0, input_attachment_index = 1, binding = 4) uniform subpassInput inDiffuse;
layout(set=0, input_attachment_index = 2, binding = 5) uniform subpassInput inNormal;
layout(set=0, input_attachment_index = 3, binding = 6) uniform subpassInput inMaterial;
layout(set=0, binding = 7) uniform sampler2D inShadowMap;

struct Model {
	vec3 position;
	vec2 uv;
    vec3 normal;
};

layout(set=1, binding = 0) buffer VerticesBuffers {Model data[];} verticesArray[];
// layout(set=2, binding = 0) buffer IndicesBuffers {uint data[];} indicesArray[];
// layout(set=3, binding = 0) uniform sampler2D images[];

// layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColour;

#include "Lighting.glsl"

void main() {
	Model t = verticesArray[50].data[100];
    // uint t2 = indicesArray[0].data[0];

	vec3 worldPosition = subpassLoad(inPosition).rgb;
	vec4 screenPosition = scene.view * vec4(worldPosition, 1.0f);
	vec4 shadowCoords = scene.shadowMatrix * vec4(worldPosition, 1.0f);

	vec3 diffuse = subpassLoad(inDiffuse).rgb;
	vec3 normal = subpassLoad(inNormal).rgb;
	vec3 material = subpassLoad(inMaterial).rgb;

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
	
	// outColour = vec4(diffuse * Lo * shadowValue, 1.0f) ;
	outColour = vec4(t.position, 1.0f) ;

}