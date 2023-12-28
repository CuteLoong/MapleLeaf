#ifndef LGC_GLSL
#define LGC_GLSL

/** Simple linear congruential generator (LCG).

    The code uses the parameters from the book series "Numerical Recipes".
    The period is 2^32 and its state size is 32 bits.

    Note: Only for basic applications. The generator has poor statistical
    properties and is sensitive to good seeding. If many parallel generators
    are used (e.g. one per pixel) there will be significant correlation
    between the generated pseudorandom sequences. In those cases, it is
    recommended to use one of the generators with larger state.
*/

uint lcg(inout uint prev)
{
  uint LCG_A = 1664525u;
  uint LCG_C = 1013904223u;
  prev       = (LCG_A * prev + LCG_C);
  return prev & 0x00FFFFFF;
}

#endif // LGC_GLSL