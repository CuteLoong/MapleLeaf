#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 1) uniform UniformObject {
	mat4 transform;
	mat4 prevTransform;

	vec4 baseDiffuse;
	float metallic;
	float roughness;
	float ignoreFog;
	float ignoreLighting;
} object;

#if DIFFUSE_MAPPING
layout(set = 0, binding = 3) uniform sampler2D samplerDiffuse;
#endif
#if MATERIAL_MAPPING
layout(set = 0, binding = 4) uniform sampler2D samplerMaterial;
#endif
#if NORMAL_MAPPING
layout(set = 0, binding = 5) uniform sampler2D samplerNormal;
#endif

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outDiffuse;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outMaterial;

void main()
{
    vec4 diffuse = object.baseDiffuse;
	vec3 normal = normalize(inNormal);
	vec3 material = vec3(object.metallic, object.roughness, 0.0f);

#if DIFFUSE_MAPPING
	diffuse = texture(samplerDiffuse, inUV);
#endif

#if MATERIAL_MAPPING
	vec4 textureMaterial = texture(samplerMaterial, inUV);
	material.x *= textureMaterial.b;
	material.y *= textureMaterial.g;
#endif

#if NORMAL_MAPPING
	vec3 tangentNormal = texture(samplerNormal, inUV).rgb * 2.0f - 1.0f;
	
	vec3 q1 = dFdx(inPosition);
	vec3 q2 = dFdy(inPosition);
	vec2 st1 = dFdx(inUV);
	vec2 st2 = dFdy(inUV);

	vec3 N = normalize(inNormal);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	normal = TBN * tangentNormal;
#endif

    outPosition = vec4(inPosition, 1.0f);
	outDiffuse = diffuse;
	outNormal = vec4(normalize(normal), 1.0f);
	outMaterial = vec4(material, 1.0f);
}