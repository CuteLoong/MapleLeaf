#version 450

// layout(location = 0) in vec3 inPosition;

struct GPUMaterialData
{
    vec4  baseColor;
    float metalic;
    float roughness;
    uint  baseColorTex;
    uint  normalTex;
    uint  materialTex;
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

layout(location = 0) out vec4 outColour;


void main() 
{	
	GPUMaterialData materialData = materialDatas.materialData[inMaterialId];
	
	outColour = vec4(materialData.baseColor.rgb, 1.0);
	// outColour = vec4(1.0);

}