#version 460

#extension GL_ARB_viewport_array : enable
#extension GL_GOOGLE_include_directive : require

layout (triangles, invocations = 2) in;
layout (triangle_strip, max_vertices = 3) out;

#include <Misc/Camera.glsl>

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec3 inUVW[];

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outUVW;

void main()
{
    for(int i = 0; i < gl_in.length(); i++)
    {
        outPosition = inPosition[i];
        outUVW = inUVW[i];

        vec4 worldPosition = gl_in[i].gl_Position;

        mat4 viewStatic = mat4(camera.stereoView[gl_InvocationID]);
	    viewStatic[3][0] = 0.0f;
	    viewStatic[3][1] = 0.0f;
	    viewStatic[3][2] = 0.0f;

        gl_Position = camera.stereoProjection[gl_InvocationID] * viewStatic * worldPosition;

        gl_ViewportIndex = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}