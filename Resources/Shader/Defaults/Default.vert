#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform UniformScene
{
    mat4 projection;
    mat4 view;
    vec3 cameraPos;
} scene;

layout(set = 0, binding = 1) uniform UniformObject
{
    mat4 transform;
    mat4 prevTransform;

    vec4  baseDiffuse;
    float metallic;
    float roughness;
    float ignoreFog;
    float ignoreLighting;
} object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outNormal;

out gl_PerVertex {
	vec4 gl_Position;
};

void main()
{
    vec4 position = vec4(inPosition, 1.0f);
	vec4 normal = vec4(inNormal, 0.0f);

    vec4 worldPosition = object.transform * position;
    vec4 prevWorldPosition = object.prevTransform * position;
    mat3 normalMatrix = transpose(inverse(mat3(object.transform)));

    gl_Position = scene.projection * scene.view * worldPosition;
    
    outPosition = worldPosition.xyz;
	outUV = inUV;
	outNormal = normalMatrix * normalize(normal.xyz);
}