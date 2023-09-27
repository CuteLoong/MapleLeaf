#pragma once

#include "glm/gtx/hash.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>

namespace MapleLeaf {
#define M_PI 3.14159265358979323846
class Maths
{
public:
    /**
     * Combines a seed into a hash and modifies the seed by the new hash.
     * @param seed The seed.
     * @param v The value to hash.
     */
    template<typename T>
    static void HashCombine(std::size_t& seed, const T& v) noexcept
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    // Base 2 Van der Corput radical inverse
    static float radicalInverse(uint32_t i)
    {
        i = (i & 0x55555555) << 1 | (i & 0xAAAAAAAA) >> 1;
        i = (i & 0x33333333) << 2 | (i & 0xCCCCCCCC) >> 2;
        i = (i & 0x0F0F0F0F) << 4 | (i & 0xF0F0F0F0) >> 4;
        i = (i & 0x00FF00FF) << 8 | (i & 0xFF00FF00) >> 8;
        i = (i << 16) | (i >> 16);
        return float(i) * 2.3283064365386963e-10f;
    }

    static glm::vec3 hammersleyUniform(uint32_t i, uint32_t n)
    {
        glm::vec2 uv((float)i / (float)n, radicalInverse(i));

        // Map to radius 1 hemisphere
        float phi = uv.y * 2.0f * (float)M_PI;
        float t   = 1.0f - uv.x;
        float s   = std::sqrt(1.0f - t * t);
        return glm::vec3(s * std::cos(phi), s * std::sin(phi), t);
    }

    inline glm::vec3 hammersleyCosine(uint32_t i, uint32_t n)
    {
        glm::vec2 uv((float)i / (float)n, radicalInverse(i));

        // Map to radius 1 hemisphere
        float phi = uv.y * 2.0f * (float)M_PI;
        float t   = std::sqrt(1.0f - uv.x);
        float s   = std::sqrt(1.0f - t * t);
        return glm::vec3(s * std::cos(phi), s * std::sin(phi), t);
    }
};
}   // namespace MapleLeaf