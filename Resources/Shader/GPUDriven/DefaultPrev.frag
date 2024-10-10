#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_nonuniform_qualifier : enable

#extension GL_GOOGLE_include_directive : require

#include <Misc/Camera.glsl>

layout(location = 0) in vec4 hPos;
layout(location = 1) in vec4 prevHPos;

layout(location = 0) out vec4 outMotionVetcor;


void main() 
{
	vec3 vPos = (hPos.xyz / hPos.w + 1.0f) * 0.5f;
	vec3 prevVPos = (prevHPos.xyz / prevHPos.w + 1.0f) * 0.5f;
    vec2 curPixelUV = ivec2(vPos.xy * camera.pixelSize.xy) * camera.pixelSize.zw; 
    vec2 prevPixelUV = ivec2(prevVPos.xy * camera.pixelSize.xy) * camera.pixelSize.zw;

	vec2 mv = curPixelUV - prevPixelUV;
	mv.y = -mv.y;
	outMotionVetcor = vec4(mv, 1.0f);
}