#pragma once

#include "glm/glm.hpp"

namespace MapleLeaf {
class CPUSampleGenerator
{
public:
    virtual ~CPUSampleGenerator() = default;

    virtual uint32_t  getSampleCount() const      = 0;
    virtual void      reset(uint32_t startID = 0) = 0;
    virtual glm::vec2 next()                      = 0;

protected:
    CPUSampleGenerator() = default;
};
}   // namespace MapleLeaf