#pragma once

#include "Time.hpp"
#include <stdint.h>

namespace MapleLeaf {
class ElapsedTime
{
public:
    explicit ElapsedTime(const Time& interval = -1s);

    uint32_t GetElapsed();

    const Time& GetStartTime() const { return startTime; }
    void        SetStartTime(const Time& startTime) { this->startTime = startTime; }

    const Time& GetInterval() const { return interval; }
    void        SetInterval(const Time& interval) { this->interval = interval; }

private:
    Time startTime;
    Time interval;
};
}   // namespace Maple