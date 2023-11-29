#include "HaltonSamplePattern.hpp"

namespace MapleLeaf {
/**
 * @brief Construct a new Halton Sample Pattern:: Halton Sample Pattern object
 *
 * @param index index Index of the queried element, starting from 0.
 * @param base Base for the digit inversion. Should be the next unused prime number.
 * @return float
 */
float halton(uint32_t index, uint32_t base)
{
    // Reversing digit order in the given base in floating point.
    float result = 0.0f;
    float factor = 1.0f;

    for (; index > 0; index /= base) {
        factor /= base;
        result += factor * (index % base);
    }

    return result;
}

HaltonSamplePattern::HaltonSamplePattern(uint32_t sampleCount)
{
    sampleCount = sampleCount;
    curSample   = 0;
}

glm::vec2 HaltonSamplePattern::next()
{
    glm::vec2 value = {halton(curSample, 2), halton(curSample, 3)};

    // Modular increment.
    ++curSample;
    if (sampleCount != 0) {
        curSample = curSample % sampleCount;
    }

    // Map the result so that [0, 1) maps to [-0.5, 0.5) and 0 maps to the origin.
    return glm::fract(value + 0.5f) - 0.5f;
}

}   // namespace MapleLeaf