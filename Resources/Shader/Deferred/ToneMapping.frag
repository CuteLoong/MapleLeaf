#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

layout(set=0, binding = 0) uniform UniformToneMapping {
	float exposure;
    float gamma;
} toneMapping;

layout(input_attachment_index=0, set=0, binding = 1) uniform subpassInput ResolvedImage;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = toneMapping.exposure * subpassLoad(ResolvedImage);
    outColor = vec4(pow(color.rgb, vec3(1.0 / toneMapping.gamma)), color.a);
}