#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct GPUInstanceData
{
    mat4 modelMatrix;
    mat4 prevModelMatrix;
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
layout(location = 1) out vec3 outPrevPosition;

void main()
{
    uint instanceIndex = gl_InstanceIndex;
    GPUInstanceData instanceData = instanceDatas.instanceData[nonuniformEXT(instanceIndex)];

    vec4 position = vec4(inPosition, 1.0f);

    vec4 worldPosition = instanceData.modelMatrix * position;
    vec4 prevWorldPosition = instanceData.prevModelMatrix * position;

    gl_Position = prevWorldPosition;

    outPosition = worldPosition.xyz;
    outPrevPosition = prevWorldPosition.xyz;
}