#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

layout(set=0, binding=0) uniform UniformScene
{
	mat4 projection[2];
	mat4 view[2];
    vec4 zBufferParams;
} scene;

layout(set=0, binding = 1) uniform sampler2D inDepth;
layout(set=0, binding = 2) uniform sampler2D inPosition;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColour;

float Linear01Depth(float z)
{
    return 1.0 / (scene.zBufferParams.x * z + scene.zBufferParams.y);
}

float LinearEyeDepth(float z)
{
    return 1.0 / (scene.zBufferParams.z * z + scene.zBufferParams.w);
}

void main()
{
    int viewIndex = inUV.x < 0.5f ? 0 : 1;
    int inverseViewIndex = 1 - viewIndex;

    vec2 uv = vec2(inUV.x, 1.0f - inUV.y);
    vec2 screenUV = vec2(uv.x / 2.0f + 0.5f * viewIndex, uv.y);

    vec3 worldPosition = texture(inPosition, uv).rgb;
    vec4 otherEyeProj = scene.projection[inverseViewIndex] * scene.view[inverseViewIndex] * vec4(worldPosition, 1.0f);
    otherEyeProj /= otherEyeProj.w;

    vec2 otherEyeUV = vec2(otherEyeProj.x, -otherEyeProj.y) * 0.5f + 0.5f; 
    otherEyeUV = vec2(otherEyeUV.x / 2.0f + 0.5f * float(inverseViewIndex), otherEyeUV.y);

    float otherEyeDepth = texture(inDepth, otherEyeUV).r;

    int mask = abs(Linear01Depth(otherEyeDepth) - Linear01Depth(otherEyeProj.z)) > 0.005 ? 1 : 0;

    outColour = vec4(mask, mask, mask, 1.0f);
}