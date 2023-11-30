#pragma once

#include "CPUSampleGenerator.hpp"
#include <memory>

namespace MapleLeaf {
class HaltonSamplePattern : public CPUSampleGenerator
{
public:
    virtual ~HaltonSamplePattern() = default;

    /**
     * @brief Construct a new Halton Sample Pattern object
     *
     * @param sampleCount The pattern repeats every 'sampleCount' samples. Zero means no repeating.
     * @return New object, or throws an exception on error.
     */
    static std::shared_ptr<HaltonSamplePattern> Create(uint32_t sampleCount = 0)
    {
        return std::shared_ptr<HaltonSamplePattern>(new HaltonSamplePattern(sampleCount));
    }

    uint32_t  getSampleCount() const override { return sampleCount; }
    void      reset(uint32_t startID = 0) override { curSample = 0; }
    glm::vec2 next() override;

protected:
    HaltonSamplePattern(uint32_t sampleCount);

    uint32_t curSample = 0;
    uint32_t sampleCount;
};
}   // namespace MapleLeaf