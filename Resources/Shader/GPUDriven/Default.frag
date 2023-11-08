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
layout(location = 4) in flat uint inInstanceID;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outDiffuse;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outMaterial;
layout(location = 4) out vec4 outInstanceID;

void main() 
{	
	GPUMaterialData materialData = materialDatas.materialData[inMaterialId];

	vec4 diffuse = materialData.baseColor;
	vec3 normal = normalize(inNormal);
	vec3 material = vec3(materialData.metallic, materialData.roughness, 0.0f);

	int baseColorTex = materialData.baseColorTex;
	int normalTex = materialData.normalTex;
	int materialTex = materialData.materialTex;

	if(baseColorTex != -1) diffuse = texture(ImageSamplers[baseColorTex], inUV);

	if(materialTex != -1) {
		vec4 textureMaterial = texture(ImageSamplers[materialTex], inUV);
		material.x *= textureMaterial.r;
		material.y *= textureMaterial.g;
	}

	if(normalTex != -1) {
		vec3 tangentNormal = texture(ImageSamplers[normalTex], inUV).rgb * 2.0f - 1.0f;
	
		vec3 q1 = dFdx(inPosition);
		vec3 q2 = dFdy(inPosition);
		vec2 st1 = dFdx(inUV);
		vec2 st2 = dFdy(inUV);

		vec3 N = normalize(inNormal);
		vec3 T = normalize(q1 * st2.t - q2 * st1.t);
		vec3 B = -normalize(cross(N, T));
		mat3 TBN = mat3(T, B, N);

		normal = TBN * tangentNormal;
	}

	outPosition = vec4(inPosition, 1.0f);
	outDiffuse = diffuse;
	outNormal = vec4(normalize(normal), 1.0f);
	outMaterial = vec4(material, 1.0f);
	outInstanceID = vec4(inInstanceID, inInstanceID, inInstanceID, inInstanceID);
}