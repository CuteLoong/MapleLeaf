#pragma once
#include "ElapsedTime.hpp"
#include "Module.hpp"
#include "NonCopyable.hpp"
#include "Time.hpp"
#include "TypeInfo.hpp"
#include <cmath>
#include <map>
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

    explicit Engine(std::string argv0, ModuleFilter&& moduleFilter = {});
    ~Engine();

    const Time& GetDelta() const { return deltaUpdate.change; }

private:
    void CreateModule(Module::TRegistryMap::const_iterator it, const ModuleFilter& filter);
    void DestroyModule(TypeId id);
    void UpdateStage(Module::Stage stage);

    static Engine* Instance;

    std::string argv0;

    std::map<TypeId, std::unique_ptr<Module>>    modules;
    std::map<Module::Stage, std::vector<TypeId>> moduleStages;

    Delta deltaUpdate;
};
}   // namespace MapleLeaf