#ifndef MISC_PARAMETERS_GLSL
#define MISC_PARAMETERS_GLSL

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

struct Vertex {
    vec3 position;
    vec2 texCoord;
    vec3 normal;
};

struct Material {
    vec4  baseColor;
    float metallic;
    float roughness;
    int   baseColorTex;
    int   normalTex;
    int   materialTex;
};

struct SceneDescription {
    uint64_t vertexAddress;         // Address of the Vertex buffer
    uint64_t indexAddress;          // Address of the index buffer
    uint64_t materialAddress;       // Address of the material buffer
    uint64_t instanceInfoAddress;  // Address of the triangle material index buffer
};

struct InstanceData
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

#endif