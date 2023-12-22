#ifndef RAY_TRACING_COMMON_GLSL
#define RAY_TRACING_COMMON_GLSL

struct HitPayLoad
{
    vec3 Lo;
    int depth;
    vec3 accBRDF;
    int done;
    vec4 nextOrigin;
    vec4 nextDir;
    vec2 randomXi;
    float accPDF;
    uint randomSeed;
};

#endif // RAY_TRACING_COMMON_GLSL