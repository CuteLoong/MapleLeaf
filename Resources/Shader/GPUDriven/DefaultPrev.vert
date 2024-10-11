#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_nonuniform_qualifier : enable

#extension GL_GOOGLE_include_directive : require

#include <Misc/Camera.glsl>

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

layout(location = 0) out vec4 outHPosition;
layout(location = 1) out vec4 outPrevHPosition;

void main()
{
    uint instanceIndex = gl_InstanceIndex;
    GPUInstanceData instanceData = instanceDatas.instanceData[nonuniformEXT(instanceIndex)];

    vec4 position = vec4(inPosition, 1.0f);

    vec4 worldPosition = instanceData.modelMatrix * position;
    vec4 prevWorldPosition = instanceData.prevModelMatrix * position;

    mat4 preProjection = camera.prevProjection;
    mat4 preView = camera.prevView;

    mat4 projection = GetProjection();
    mat4 view = GetView();

    gl_Position = preProjection * preView * prevWorldPosition;

    outHPosition = projection * view * worldPosition;
    outPrevHPosition = preProjection * preView * prevWorldPosition;
}