#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

//layout(constant_id = 0) const int SSAO_KERNEL_SIZE = 64;
//layout(constant_id = 1) const float SSAO_RADIUS = 0.5f;

layout(set=0, binding=0) uniform UniformScene
{
    vec4 kernel[SSAO_KERNEL_SIZE];

	mat4 projection[2];
	mat4 view[2];
	vec4 cameraPosition[2];
} scene;

layout(set=0, binding = 1) uniform sampler2D inNormal;
layout(set=0, binding = 2) uniform sampler2D inPosition;
layout(set=0, binding = 3) uniform sampler2D samplerNoise;

layout(location = 0) out vec4 outColour;

layout(location = 0) in vec2 inUV;

void main()
{
    int viewIndex = inUV.x < 0.5f ? 0 : 1;

    vec2 uv = vec2(inUV.x, 1.0f - inUV.y);
    vec3 worldPosition = texture(inPosition, uv).rgb;
    vec3 normal = texture(inNormal, uv).rgb;

    float originDist = length(worldPosition - scene.cameraPosition[viewIndex].xyz);

    vec2 stereoNoiseUV = vec2(uv.x / 2.0f + 0.5f * float(viewIndex), uv.y);
    vec3 randDir = texture(samplerNoise, stereoNoiseUV).rgb * 2.0f - 1.0f;

    vec3 tangent = normalize(randDir - normal * dot(randDir, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // Calculate occlusion value.
	float occlusion = 0.0f;
    for(uint i = 0; i < SSAO_KERNEL_SIZE; i++) {
        vec3 samplePos = TBN * scene.kernel[i].xyz;
        samplePos = samplePos * SSAO_RADIUS + worldPosition;

        float sampleDepth = length(samplePos - scene.cameraPosition[viewIndex].xyz);

        vec4 samplePosProj = scene.projection[viewIndex] * scene.view[viewIndex] * vec4(samplePos, 1.0f);
        samplePosProj /= samplePosProj.w;

        vec2 sampleUV = vec2(samplePosProj.x, -samplePosProj.y) * 0.5f + 0.5f; 
        sampleUV = vec2(sampleUV.x / 2.0f + 0.5f * float(viewIndex), sampleUV.y);  // This is mono position but we sample on stereo position, so uv.x must division 2.0f
        float sceneDepth = length(texture(inPosition, sampleUV).xyz - scene.cameraPosition[viewIndex].xyz);

        float rangeCheck = step(abs(sampleDepth - sceneDepth), SSAO_RADIUS);
        occlusion += step(sceneDepth, sampleDepth) * rangeCheck;
    }

    float factor = 1 - (occlusion / float(SSAO_KERNEL_SIZE));
    outColour = vec4(factor, factor, factor, 1.0f);
}