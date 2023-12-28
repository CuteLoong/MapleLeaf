#ifndef XOSHIRO_GLSL
#define XOSHIRO_GLSL

uint rotl(const uint x, int k)
{
    return (x << k) | (x >> (32 - k));
}

uint nextRandom(inout uvec4 rng) 
{
    const uint32_t result_starstar = rotl(rng[0] * 5, 7) * 9;
	const uint32_t t = rng[1] << 9;

    rng[2] ^= rng[0];
    rng[3] ^= rng[1];
    rng[1] ^= rng[2];
    rng[0] ^= rng[3];

    rng[2] ^= t;
    rng[3] = rotl(rng[3], 11);

    return result_starstar;
}

#endif // XOSHIRO_GLSL