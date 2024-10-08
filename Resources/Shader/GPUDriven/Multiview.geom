#version 450

#extension GL_ARB_viewport_array : enable
#extension GL_GOOGLE_include_directive : require

layout (triangles, invocations = 2) in;
layout (triangle_strip, max_vertices = 3) out;

#include <Misc/Camera.glsl>

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec2 inUV[];
layout(location = 2) in vec3 inNormal[];
layout(location = 3) in flat uint inMaterialId[];
layout(location = 4) in flat uint inInstanceId[];
layout(location = 5) in vec3 inPrevPosition[];


layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out flat uint outMaterialId;
layout(location = 4) out flat uint outInstanceId;
layout(location = 5) out vec4 hPos;
layout(location = 6) out vec4 prevHPos;

void main()
{
    for(int i = 0; i < gl_in.length(); i++)
    {
        outPosition = inPosition[i];
        outUV = inUV[i];
        outNormal = inNormal[i];
        outMaterialId = inMaterialId[i];
        outInstanceId = inInstanceId[i];

        hPos = camera.stereoProjection[gl_InvocationID] * camera.stereoView[gl_InvocationID] * vec4(inPosition[i], 1.0f);
        prevHPos = camera.prevStereoProjection[gl_InvocationID] * camera.prevStereoView[gl_InvocationID] * vec4(inPrevPosition[i], 1.0f);

        vec4 worldPosition = gl_in[i].gl_Position;
        gl_Position = camera.stereoProjection[gl_InvocationID] * camera.stereoView[gl_InvocationID] * worldPosition;

        gl_ViewportIndex = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}