#ifndef HAMMERSLEYSEQUENCE_GLSL
#define HAMMERSLEYSEQUENCE_GLSL

float radicalInverse(uint i)
{
    i = (i & 0x55555555) << 1 | (i & 0xAAAAAAAA) >> 1;
    i = (i & 0x33333333) << 2 | (i & 0xCCCCCCCC) >> 2;
    i = (i & 0x0F0F0F0F) << 4 | (i & 0xF0F0F0F0) >> 4;
    i = (i & 0x00FF00FF) << 8 | (i & 0xFF00FF00) >> 8;
    i = (i << 16) | (i >> 16);
    return float(i) * 2.3283064365386963e-10f;
}

uint HaltonSequence(uint Index, uint base)
{
	uint result = 0;
	uint f = 1;
	uint i = Index;
	
	while (i > 0) {
		result += (f / base) * (i % base);
		i = uint(floor(i / base));
	}
	return result;
}

vec2 getHammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), radicalInverse(i));
}

vec2 getHammersley(uint i, uint N, uvec2 scramble)
{
    return vec2(float(i) / float(N), radicalInverse(i ^ (scramble.x | (scramble.y << 16))));
}

#endif