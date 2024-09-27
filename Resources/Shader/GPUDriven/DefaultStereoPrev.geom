#version 450

#extension GL_ARB_viewport_array : enable
#extension GL_GOOGLE_include_directive : require

layout (triangles, invocations = 2) in;
layout (triangle_strip, max_vertices = 3) out;

#include <Misc/Camera.glsl>

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec3 inPrevPosition[];


layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec4 hPos;
layout(location = 2) out vec4 prevHPos;

void main()
{
    for(int i = 0; i < gl_in.length(); i++)
    {
        outPosition = inPosition[i];

        hPos = camera.stereoProjection[gl_InvocationID] * camera.stereoView[gl_InvocationID] * vec4(inPosition[i], 1.0f);
        prevHPos = camera.prevStereoProjection[gl_InvocationID] * camera.prevStereoView[gl_InvocationID] * vec4(inPrevPosition[i], 1.0f);

        vec4 worldPosition = gl_in[i].gl_Position;
        gl_Position = camera.prevStereoProjection[gl_InvocationID] * camera.prevStereoView[gl_InvocationID] * worldPosition;

        gl_ViewportIndex = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}