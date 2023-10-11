#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

layout(set=0, binding=0) uniform UniformScene
{
    vec4 pixelSize; // camera's pixelWidth, pixelHeight, 1.0 / pixelWidth, 1.0f / pixelHeight
} scene;

layout(set=0, binding=1) uniform UniformGaussian {
    int radius;
} uniformGaussian;

layout (set = 0, binding = 2) buffer BufferWeight {
    float weights[]; 
} bufferWeight;

layout(set=0, binding = 3) uniform sampler2D inTexture;

layout(location = 0) out vec4 outColour;
layout(location = 0) in vec2 inUV;

void main()
{
    vec2 uv = vec2(inUV.x, 1.0f - inUV.y);
    vec4 fragColor = vec4(0.0f);

    const int offset = -uniformGaussian.radius / 2;

    for(int x = 0; x < uniformGaussian.radius; x++) {
        for(int y = 0; y < uniformGaussian.radius; y++) {
            int index = x * uniformGaussian.radius + y;
            vec2 offsetUV = vec2(offset + x, offset + y) * scene.pixelSize.zw;
            fragColor += texture(inTexture, uv + offsetUV).xyzw * bufferWeight.weights[index];
        }
    }
    outColour = fragColor;
}