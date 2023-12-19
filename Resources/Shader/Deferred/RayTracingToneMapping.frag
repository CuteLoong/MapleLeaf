#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

layout(set=0, binding = 0) uniform UniformToneMapping {
	float exposure;
    float gamma;
} toneMapping;

layout(set=0, binding = 1) uniform sampler2D RayTracingTarget;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

vec3 toneMapAces(vec3 color)
{
    // Cancel out the pre-exposure mentioned in
    // https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
    color *= 0.6;

    float A = 2.51;
    float B = 0.03;
    float C = 2.43;
    float D = 0.59;
    float E = 0.14;

    color = clamp((color*(A*color+B))/(color*(C*color+D)+E), 0.0f, 1.0f);
    return color;
}

vec3 toneMap(vec3 color) {
    return toneMapAces(color);
}

void main() {
    vec2 uv = inUV;
    vec4 color = texture(RayTracingTarget, uv);

    vec3 finalColor = color.rgb;

    // finalColor = toneMap(finalColor);

    outColor = vec4(finalColor, color.a);
}