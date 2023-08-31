#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(push_constant) uniform PushObject {
	mat4 mvp;
} object;

layout(location = 0) in vec3 inPosition;
// layout(location = 1) in vec2 inUV;
// layout(location = 2) in vec3 inNormal;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() {
	gl_Position = object.mvp * vec4(inPosition, 1.0f);
	gl_Position.z = gl_Position.z * 0.5 + 0.5;
}