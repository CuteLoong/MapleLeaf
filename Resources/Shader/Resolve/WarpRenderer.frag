#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 inUV;

layout(set=0, binding = 1) uniform sampler2D LightingMap;

layout(location = 0) out vec4 outColor;

void main()
{
	vec2 uv = vec2(inUV.x, 1.0f - inUV.y);
    vec3 lighting = texture(LightingMap, uv).xyz;

    outColor = vec4(lighting, 1.0f);
}