#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) uniform UniformScene
{
    mat4 projection;
    mat4 view;
    vec3 cameraPos;
} scene;

// struct IndirectCommand {
//     uint indexCount;
//     uint instanceCount;
//     uint firstIndex;
//     int  vertexOffset;
//     uint firstInstance;
// };

// layout(set = 0, binding = 1) buffer DrawCommandBuffer 
// {
//     IndirectCommand commands[];
// } drawCommandBuffer;

struct GPUInstanceData
{
    mat4 modelMatrix;
    vec3 AABBLocalMin;
    uint indexCount;
    vec3 AABBLocalMax;
    uint indexOffset;
    uint vertexCount;
    uint vertexOffset;
    uint instanceID;
    uint materialID;
};

layout(set = 0, binding = 1) buffer InstanceDatas
{
    GPUInstanceData instanceData[];
} instanceDatas;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out flat uint outMaterialId;

void main()
{
    uint instanceIndex = gl_InstanceIndex;
    GPUInstanceData instanceData = instanceDatas.instanceData[nonuniformEXT(instanceIndex)];
    uint materialId = instanceData.materialID;

    vec4 position = vec4(inPosition, 1.0f);
    vec4 normal = vec4(inNormal, 0.0f);

    vec4 worldPosition = instanceData.modelMatrix * position;
    mat3 normalMatrix = transpose(inverse(mat3(instanceData.modelMatrix)));

    gl_Position = scene.projection * scene.view * worldPosition;

    outPosition = worldPosition.xyz;
    outUV = inUV;
	outNormal = normalMatrix * normalize(normal.xyz);
    outMaterialId = materialId;
}