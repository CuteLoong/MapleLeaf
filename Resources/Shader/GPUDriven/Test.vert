#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform UniformScene
{
    mat4 projection;
    mat4 view;
    vec3 cameraPos;
} scene;

layout(set = 0, binding = 1) uniform UniformObject
{
    mat4 transform;

    vec4  baseDiffuse;
    float metallic;
    float roughness;
    float ignoreFog;
    float ignoreLighting;
} object;

// struct Model {
// 	vec3 position;
// 	vec2 uv;
//     vec3 normal;
// };

// layout(set=1, binding = 0) buffer VerticesBuffers {Model data[]} verticesArray;
// layout(set=2, binding = 0) buffer IndicesBuffers {uint data[]} indicesArray;
// layout(set=3, binding = 0) uniform sampler2D images[];


void main()
{
    Model t = verticesArray[0][0];
    // uint t2 = indicesArray[0][0];

    vec4 position = vec4(0.0f);
	vec4 normal = vec4(1.0f, 0.0f, 0.0f, 0.0f);

    vec4 worldPosition = object.transform * position;
    mat3 normalMatrix = transpose(inverse(mat3(object.transform)));

    gl_Position = scene.projection * scene.view * worldPosition;
}