#pragma once
#include "ElapsedTime.hpp"
#include "NonCopyable.hpp"
#include "Time.hpp"
#include <cmath>
#include <string>
namespace MapleLeaf {
class Delta
{
public:
    Time currentFrameTime;
    Time lastFrameTime;
    Time change;

    void Update()
    {
        currentFrameTime = Time::Now();
        change           = currentFrameTime - lastFrameTime;
        lastFrameTime    = currentFrameTime;
    }
};

class ChangePerSecond
{
public:
    uint32_t valueTemp = 0, value = 0;
    Time     valueTime;

    void Update(const Time& time)
    {
        valueTemp++;

        if (std::floor(time.AsSeconds()) > std::floor(valueTime.AsSeconds())) {
            value     = valueTemp;
            valueTemp = 0;
        }

        valueTime = time;
    }
};

class Engine : NonCopyable
{
public:
    static Engine* Get() { return Instance; }

    explicit Engine(std::string argv0);
    ~Engine();

    const Time& GetDelta() const { return deltaUpdate.change; }

private:
    std::string argv0;

    static Engine* Instance;
    Delta          deltaUpdate;
};
}   // namespace MapleLeaf