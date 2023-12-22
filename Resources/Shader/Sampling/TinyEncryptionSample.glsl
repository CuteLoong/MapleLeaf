#ifndef TINY_ENCRYPTION_SAMPLE_GLSL
#define TINY_ENCRYPTION_SAMPLE_GLSL

#include "Pseudorandom/LGC.glsl"

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