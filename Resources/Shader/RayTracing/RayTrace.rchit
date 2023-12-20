#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

layout(location = 0) rayPayloadInEXT vec3 hitValue;
hitAttributeEXT vec3 attribs;

#include <Misc/Parameters.glsl>

layout(buffer_reference, scalar) readonly buffer Vertices {Vertex v[]; };
layout(buffer_reference, scalar) readonly buffer Indices {uint i[]; };

layout(set = 0, binding = 3, scalar) uniform UniformSceneData {
	uint64_t vertexAddress;      
	uint64_t indexAddress;       
} uniformSceneData;

layout(set = 0, binding = 4) buffer InstanceDatas
{
    InstanceData instanceData[];
} instanceDatas;

layout(set = 0, binding = 5) buffer MaterialDatas
{
    Material materialData[];
} materialDatas;

layout(set = 1, binding = 0) uniform sampler2D ImageSamplers[];

void main()
{
	InstanceData instanceData = instanceDatas.instanceData[gl_InstanceCustomIndexEXT];
	Material material = materialDatas.materialData[instanceData.materialID];
	Vertices vertices = Vertices(uniformSceneData.vertexAddress);
	Indices indices = Indices(uniformSceneData.indexAddress);

	uint indexOffset = instanceData.indexOffset;
	uint vertexOffset = instanceData.vertexOffset;

	vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	ivec3 index = ivec3(indices.i[indexOffset + gl_PrimitiveID * 3], indices.i[indexOffset + gl_PrimitiveID * 3 + 1], indices.i[indexOffset + gl_PrimitiveID * 3 + 2]);
	Vertex v0 = vertices.v[vertexOffset + index.x];
	Vertex v1 = vertices.v[vertexOffset + index.y];
	Vertex v2 = vertices.v[vertexOffset + index.z];

	vec3 position = v0.position * barycentrics.x + v1.position * barycentrics.y + v2.position * barycentrics.z;
	vec3 normal = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
	vec2 texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y + v2.texCoord * barycentrics.z;

	vec3 worldPosition = vec3(gl_ObjectToWorldEXT * vec4(position, 1.0f));
	vec3 worldNormal = normalize(vec3(normal * gl_WorldToObjectEXT));

	vec4 diffuse = material.baseColor;
	vec3 roughnessMetalic = vec3(material.metallic, material.roughness, 0.0f);

	int baseColorTex = material.baseColorTex;
	int normalTex = material.normalTex;
	int materialTex = material.materialTex;

	if(baseColorTex != -1) diffuse = texture(ImageSamplers[baseColorTex], texCoord);
	if(materialTex != -1) {
		vec4 textureMaterial = texture(ImageSamplers[materialTex], texCoord);
		roughnessMetalic.x *= textureMaterial.r;
		roughnessMetalic.y *= textureMaterial.g;
	}
	if(normalTex != -1) {
		vec3 tangentNormal = texture(ImageSamplers[normalTex], texCoord).rgb * 2.0f - 1.0f;
		vec3 q1 = normalize(v0.position - v1.position);
		vec3 q2 = normalize(v2.position - v1.position);
		vec2 st1 = v0.texCoord - v1.texCoord;
		vec2 st2 = v2.texCoord - v1.texCoord;
		vec3 N = normalize(worldNormal);
		vec3 T = normalize(q1 * st2.t - q2 * st1.t);
		vec3 B = -normalize(cross(N, T));
		mat3 TBN = mat3(T, B, N);

		worldNormal = normalize(TBN * tangentNormal);
	}

  	hitValue = vec3(worldNormal);
}
