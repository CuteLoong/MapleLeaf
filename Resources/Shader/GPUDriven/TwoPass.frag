#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_nonuniform_qualifier : enable
// layout(location = 0) in vec3 inPosition;

struct GPUMaterialData
{
    vec4  baseColor;
    float metallic;
    float roughness;
    int   baseColorTex;
    int   normalTex;
    int   materialTex;
};

layout(set = 0, binding = 2) buffer MaterialDatas
{
    GPUMaterialData materialData[];
} materialDatas;

layout(set = 1, binding = 0) uniform sampler2D ImageSamplers[];

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in flat uint inMaterialId;

// layout(location = 0) out vec4 outPosition;
// layout(location = 1) out vec4 outDiffuse;
// layout(location = 2) out vec4 outNormal;
// layout(location = 3) out vec4 outMaterial;

void main() 
{	
	
}