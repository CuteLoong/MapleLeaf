#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 inUV;

layout(set=0, binding = 1) uniform sampler2D inDepth;
layout(set=0, binding = 2) uniform sampler2D inNormal;
layout(set=0, binding = 3) uniform sampler2D inMaterial;
layout(set=0, binding = 4) uniform sampler2D inDiffuse;
layout(set=0, binding = 5) uniform sampler2D LightingMap;
layout(set=0, binding = 6) uniform sampler2D ReflectionColorMap;
layout(set=0, binding = 7) uniform sampler2D PreIntegratedBRDF;
layout(set=0, binding = 8) uniform sampler2D GlossyMV;

#include <Misc/Camera.glsl>
#include <Misc/Utils.glsl>
#include <Misc/Constants.glsl>

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outIndirectLighting;

vec3 LookUpPreIntegratedBRDF(float NoV, float roughness)
{
    return texture(PreIntegratedBRDF, vec2(NoV, roughness)).xyz;
}

float Luminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

void ResolverAABB(sampler2D currColor, float Sharpness, float ExposureScale, float AABBScale, vec2 uv, vec2 TexelSize, inout float Variance, inout vec4 MinColor, inout vec4 MaxColor, inout vec4 FilterColor)
{
    const vec2 offset[9] = {vec2(-1.0, -1.0), vec2(0.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 0.0), vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(-1.0, 1.0), vec2(0.0, 1.0), vec2(1.0, 1.0)};
    vec4 SampleColors[9];

    for(int i = 0; i < 9; i++)
    {
        vec2 sampleUV = uv + offset[i] * TexelSize;
        SampleColors[i] = textureLod(currColor, sampleUV, 0);
    }

    vec4 m1 = vec4(0.0f);
    vec4 m2 = vec4(0.0f);

    for(uint x = 0; x < 9; x++) 
    {
        m1 += SampleColors[x];
        m2 += SampleColors[x] * SampleColors[x];
    }

    vec4 mean = m1 / 9.0f;
    vec4 stddev = sqrt((m2 / 9.0) - mean * mean);

    MinColor = mean - AABBScale * stddev;
    MaxColor = mean + AABBScale * stddev;

    FilterColor = SampleColors[4];
    MinColor = min(MinColor, FilterColor);
    MaxColor = max(MaxColor, FilterColor);

    float TotalVariation = 0.0f;
    for(uint x = 0; x < 9; x++)
    {
        TotalVariation += pow(Luminance(SampleColors[x].xyz) - Luminance(mean.xyz), 2.0f);
    }

    Variance = clamp((TotalVariation / 9.0f) * 256.0f, 0.0f, 1.0f);
    Variance *= FilterColor.w;
}


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

void main()
{
	vec2 uv = vec2(inUV.x, 1.0f - inUV.y);
    int viewIndex = inUV.x < 0.5f ? 0 : 1;

    vec3 glossyMV = texture(GlossyMV, uv).xyz;

    vec2 anotherUV = uv + glossyMV.xy;
    float curReflectDepth = glossyMV.z;
    float anotherReflectDepth = texture(GlossyMV, anotherUV).z;
    bool useAnother = true;

    if(abs(curReflectDepth - anotherReflectDepth) > 0.01f) useAnother = false;

    vec3 normalWS = texture(inNormal, uv).xyz;

    vec4 ssrColor = texture(ReflectionColorMap, uv);

    float roughness = texture(inMaterial, uv).g;
    float metallic = texture(inMaterial, uv).r;
    vec3 diffuseColor = texture(inDiffuse, uv).xyz;
    vec3 F0 = vec3(0.04f);
    F0 = mix(F0, diffuseColor, metallic);

    vec3 originWS = StereoWorldSpacePosAtScreenUV(uv);
    vec3 eyeWS = camera.cameraStereoPosition[viewIndex].xyz;
    vec3 rayWS = normalize(originWS - eyeWS);

    float NoV = clamp(dot(normalWS, -rayWS), 0.0f, 1.0f);
    vec3 brdf = LookUpPreIntegratedBRDF(NoV, roughness);

    vec3 dfg = F0 * brdf.x + brdf.y;

    vec3 reflectionColor = vec3(0.0f);
    if(useAnother) {
        float SSRVariation = 0.0f;
        vec4 SSRCurColor = vec4(0.0f);
        vec4 SSRMinColor = vec4(0.0f);
        vec4 SSRMaxColor = vec4(0.0f);

        ResolverAABB(ReflectionColorMap, 0.0f, 10.0f, 1.25f, uv, camera.pixelSize.zw, SSRVariation, SSRMinColor, SSRMaxColor, SSRCurColor);
        vec4 ssrColor2 = texture(ReflectionColorMap, anotherUV);

        ssrColor2.xyz = clamp(ssrColor2.xyz, SSRMinColor.xyz, SSRMaxColor.xyz);

        reflectionColor = ((ssrColor.xyz * ssrColor.w + ssrColor2.xyz * ssrColor2.w) * 0.5f) * dfg;
    }
    else {
        reflectionColor = ssrColor.xyz * ssrColor.w * dfg;
    }
        
    // vec3 reflectionColor = ssrColor.xyz * ssrColor.w * dfg;

    vec3 lighting = texture(LightingMap, uv).xyz;

    outColor = vec4(lighting + reflectionColor * 2.0f, 1.0f);
    // outColor = vec4(reflectionColor * 2.0f, 1.0f);
    reflectionColor = toneMap(reflectionColor  * 2.0f);

    outIndirectLighting = vec4(pow(reflectionColor, vec3(1.0f / 2.2f)), 1.0f);
}