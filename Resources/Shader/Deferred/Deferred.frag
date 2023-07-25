#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

layout(binding = 0) uniform UniformScene {
	mat4 view;
	vec3 cameraPosition;
} scene;

layout(input_attachment_index = 0, binding = 1) uniform subpassInput inPosition;
layout(input_attachment_index = 1, binding = 2) uniform subpassInput inDiffuse;
layout(input_attachment_index = 2, binding = 3) uniform subpassInput inNormal;
layout(input_attachment_index = 3, binding = 4) uniform subpassInput inMaterial;


layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColour;

void main() {
	vec3 worldPosition = subpassLoad(inPosition).rgb;
	vec4 screenPosition = scene.view * vec4(worldPosition, 1.0f);

	vec3 diffuse = subpassLoad(inDiffuse).rgb;
	// vec3 normal = texture(samplerNormal, inUV).rgb;
	// vec3 material = texture(samplerMaterial, inUV).rgb;

	// float metallic = material.r;
	// float roughness = material.g;
	// bool ignoreFog = material.b == (1.0f / 3.0f) || material.b == (3.0f / 3.0f);
	// bool ignoreLighting = material.b == (2.0f / 3.0f) || material.b == (3.0f / 3.0f);
	
	// vec3 N = normalize(normal);
	// vec3 V = normalize(scene.cameraPosition - worldPosition);
	// vec3 R = reflect(-V, N); 

	
	outColour = vec4(diffuse, 1.0f);
}