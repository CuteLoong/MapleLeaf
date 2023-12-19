#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

layout(location = 0) rayPayloadInEXT vec3 hitValue;
// hitAttributeEXT vec3 attribs;

// #include <Misc/Parameters.glsl>

// layout(buffer_reference, scalar) buffer Vertices {Vertex v[]; };
// layout(std430, buffer_reference, scalar) buffer Indices {int i[]; };

// layout(set = 0, binding = 3) buffer InstanceDatas
// {
//     GPUInstanceData instanceData[];
// } instanceDatas;

// layout(set = 0, binding = 4, scalar) uniform UniformSceneData {
// 	uint64_t vertexAddress;      
// 	uint64_t indexAddress;       
// 	uint64_t materialAddress;    
// 	uint64_t instanceInfoAddress;
// } uniformSceneData;

void main()
{
	// GPUInstanceData instanceData = instanceDatas.instanceData[gl_InstanceCustomIndexEXT];
	// Vertices vertices = Vertices(uniformSceneData.vertexAddress);
	// Indices indices = Indices(uniformSceneData.indexAddress);

  	hitValue = vec3(1.0f);
}
