#version 450

#extension GL_ARB_viewport_array : enable
#extension GL_GOOGLE_include_directive : require

layout (triangles, invocations = 2) in;
layout (triangle_strip, max_vertices = 3) out;

#include <Misc/Camera.glsl>

void main()
{
    for(int i = 0; i < gl_in.length(); i++)
    {
        vec4 worldPosition = gl_in[i].gl_Position;
        gl_Position = camera.stereoProjection[gl_InvocationID] * camera.stereoView[gl_InvocationID] * worldPosition;

        gl_ViewportIndex = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}