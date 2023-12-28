#ifndef TINY_ENCRYPTION_SAMPLE_GLSL
#define TINY_ENCRYPTION_SAMPLE_GLSL

#include "Pseudorandom/LGC.glsl"

/** Tiny uniform random sample generator.

    This generator has only 32 bit state and sub-optimal statistical properties.
    Do not use for anything critical; correlation artifacts may be prevalent.
*/

uvec2 blockCipherTEA(uint v0, uint v1, uint iterations)
{
    uint sum = 0;
    const uint delta = 0x9e3779b9;
    const uint k[4] = { 0xa341316c, 0xc8013ea4, 0xad90777d, 0x7e95761e }; // 128-bit key.
    for (uint i = 0; i < iterations; i++)
    {
        sum += delta;
        v0 += ((v1 << 4) + k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + k[1]);
        v1 += ((v0 << 4) + k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + k[3]);
    }
    return uvec2(v0, v1);
}

uint TinyEncryptionInit(uint val0, uint val1)
{
  uint v0 = val0;
  uint v1 = val1;
  uint s0 = 0;

  for(uint n = 0; n < 16; n++)
  {
    s0 += 0x9e3779b9;
    v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
    v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
  }

  return v0;
}

float TinyEncryptionRandom(inout uint prev)
{
  return (float(lcg(prev)) / float(0x01000000));
}

#endif //TINY_ENCRYPTION_SAMPLE_GLSL