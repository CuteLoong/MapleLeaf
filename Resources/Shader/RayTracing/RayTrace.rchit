#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include <Misc/RayTracingCommon.glsl>

layout(location = 0) rayPayloadInEXT HitPayLoad prd;
hitAttributeEXT vec3 attribs;

#include <Misc/Parameters.glsl>

struct PointLight {
	vec4 color;
	vec3 position;
	float pad;
	vec3 attenuation;
	float pad1;
};

struct DirectionalLight {
	vec4 color;
	vec3 direction;
	float pad;
};

layout(buffer_reference, scalar) readonly buffer Vertices {Vertex v[]; };
layout(buffer_reference, scalar) readonly buffer Indices {uint i[]; };

layout(set = 0, binding = 4, scalar) uniform UniformSceneData {
	uint64_t vertexAddress;
	uint64_t indexAddress; 
	int pointLightsCount;
	int directionalLightsCount;
} uniformSceneData;

layout(set = 0, binding = 5) buffer InstanceDatas
{
    InstanceData instanceData[];
} instanceDatas;

layout(set = 0, binding = 6) buffer MaterialDatas
{
    Material materialData[];
} materialDatas;

layout(set=0, binding = 7) buffer BufferPointLights {
	PointLight lights[];
} bufferPointLights;

layout(set=0, binding = 8) buffer BufferDirectionalLights {
	DirectionalLight lights[];
} bufferDirectionalLights;

layout(set=0, binding = 10) uniform sampler2D samplerBRDF;
layout(set=0, binding = 11) uniform samplerCube samplerIrradiance;
layout(set=0, binding = 12) uniform samplerCube samplerPrefiltered;

layout(set = 1, binding = 0) uniform sampler2D ImageSamplers[];

#include <Misc/Constants.glsl>
#include <Materials/Fresnel.glsl>
#include <Materials/BRDF.glsl>
#include <Sampling/TinyEncryptionSample.glsl>

vec3 prefilteredReflection(vec3 R, float roughness, samplerCube prefiltered) {
	float lod = roughness * float(textureQueryLevels(prefiltered));
	float lodf = floor(lod);
	float lodc = ceil(lod);
	vec3 a = textureLod(prefiltered, R, lodf).rgb;
	vec3 b = textureLod(prefiltered, R, lodc).rgb;
	return mix(a, b, lod - lodf);
}

float calcAttenuation(float distance, vec3 attenuation)
{
    return 1.0f / (attenuation.x + attenuation.y * distance + attenuation.z * distance * distance);
}

void generateBasis(vec3 N, out vec3 up, out vec3 right, out vec3 forward)
{
    up = abs(N.z) < 0.999f ? vec3(0, 0, 1) : vec3(1, 0, 0);
    right = normalize(cross(up, N));
    forward = cross(N, right);
}

vec3 localToWorld(vec3 localVector, vec3 N)
{
	vec3 up, right, forward;
	generateBasis(N, up, right, forward);

	return localVector.x * right + localVector.y * forward + localVector.z * N;
}

vec3 ImportanceSampleGGX(vec2 u, vec3 N, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float phi = M_2PI * u.x;
    float cosTheta = sqrt((1 - u.y) / (1 + (a2 - 1) * u.y));
    float sinTheta = sqrt(1 - cosTheta * cosTheta);

    // Tangent space H
    vec3 tH;
    tH.x = sinTheta * cos(phi);
    tH.y = sinTheta * sin(phi);
    tH.z = cosTheta;

    vec3 up, right, forward;
    generateBasis(N, up, right, forward);

    // World space H
    return normalize(right * tH.x + forward * tH.y + N * tH.z);
}

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
	float metallic = material.metallic;
	float roughness = material.roughness;

	int baseColorTex = material.baseColorTex;
	int normalTex = material.normalTex;
	int materialTex = material.materialTex;

	if(baseColorTex != -1) diffuse = texture(ImageSamplers[baseColorTex], texCoord);
	if(materialTex != -1) {
		vec4 textureMaterial = texture(ImageSamplers[materialTex], texCoord);
		metallic *= textureMaterial.b;
		roughness *= textureMaterial.g;
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

	// vec3 V1 = -normalize(gl_WorldRayDirectionEXT);
	vec3 V = prd.depth == 0 ? -normalize(gl_WorldRayDirectionEXT) : normalize(prd.cameraOrigin.xyz - worldPosition);

	vec3 Lo = vec3(0.0f);
	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, diffuse.rgb, metallic);
	for(int i = 1; i <= uniformSceneData.pointLightsCount; i++) {
		PointLight light = bufferPointLights.lights[i];
		vec3 L = normalize(light.position - worldPosition);
		float d = length(L);
		L = normalize(L);

		float NoL = clamp(dot(worldNormal, L), 0.0f, 1.0f);

		vec3 radiance = light.color.rgb * calcAttenuation(d, light.attenuation.xyz);

		vec3 brdf = (1.0f - metallic) * DiffuseReflectionDisney(diffuse.rgb, roughness, worldNormal, L, V) * NoL + SpecularReflectionMicrofacet(F0, roughness, worldNormal, L, V);

		// Lo += brdf * radiance;
	}

	for(int i = 1; i <= uniformSceneData.directionalLightsCount; i++) {
		DirectionalLight light = bufferDirectionalLights.lights[i];
		vec3 L = normalize(-light.direction);

		float NoL = clamp(dot(worldNormal, L), 0.0f, 1.0f);

		vec3 radiance = light.color.rgb;

		vec3 brdf = (1.0f - metallic) * DiffuseReflectionDisney(diffuse.rgb, roughness, worldNormal, L, V) * NoL + SpecularReflectionMicrofacet(F0, roughness, worldNormal, L, V);

		// Lo += brdf * radiance;
	}

	float NdotV = clamp(dot(worldNormal, V), 0.0f, 1.0f);
	vec3 R = reflect(-V, worldNormal);

	vec3 brdfPreIntegrated = texture(samplerBRDF, vec2(NdotV, roughness)).rgb;
	vec3 reflection = prefilteredReflection(R, roughness, samplerPrefiltered).rgb;	
	vec3 specular = reflection * (F0 * brdfPreIntegrated.r + brdfPreIntegrated.g);

	vec3 irradiance = texture(samplerIrradiance, worldNormal).rgb;
	vec3 diffuseLo = irradiance * (1 - metallic) * diffuse.rgb * brdfPreIntegrated.b * INV_M_PI;

	// Lo += (specular + diffuseLo);
	Lo += prd.depth == 0 ? (specular + diffuseLo) : (specular + diffuseLo) * 4.0f;

	vec2 Xi = vec2(TinyEncryptionRandom(prd.randomSeed), TinyEncryptionRandom(prd.randomSeed));

	float pdf = 1.0f;
    vec3 H = localToWorld(sampleGGX_NDF(roughness*roughness, Xi, pdf), worldNormal);
	float VoH = clamp(dot(V, H), 0.0f, 1.0f);
	pdf = pdf / (4.0f * VoH);

	prd.Lo = Lo * prd.accBRDF / prd.accPDF;
	prd.done = 0;
	prd.nextOrigin = vec4(worldPosition, 1.0f);
	prd.nextDir = vec4(normalize(reflect(-V, H)), 0.0f);
	float NoL = clamp(dot(worldNormal, R), 0.0f, 1.0f);
	prd.accBRDF *= SpecularReflectionMicrofacet(F0, roughness, worldNormal, prd.nextDir.xyz, V);
	prd.accPDF *= pdf;
}
