#pragma once
#include "App.hpp"
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

    int32_t Run();

    const std::string& GetArgv0() const { return argv0; };
    const Version&     GetVersion() const { return engineVersion; }

    App* GetApp() const { return app.get(); }
    void SetApp(std::unique_ptr<App>&& app) { this->app = std::move(app); }

    const Time& GetDelta() const { return deltaUpdate.change; }
    const Time& GetDeltaRender() const { return deltaRender.change; }

    uint32_t GetUps() const { return ups.value; }
    uint32_t GetFps() const { return fps.value; }

    void RequestClose() { running = false; }

private:
    void CreateModule(Module::TRegistryMap::const_iterator it, const ModuleFilter& filter);
    void DestroyModule(TypeId id);
    void UpdateStage(Module::Stage stage);

    static Engine* Instance;

    std::string argv0;
    Version     engineVersion;

    std::unique_ptr<App> app;

    std::map<TypeId, std::unique_ptr<Module>>    modules;
    std::map<Module::Stage, std::vector<TypeId>> moduleStages;

    float fpsLimit;
    bool  running;

    Delta           deltaUpdate, deltaRender;
    ElapsedTime     elapsedUpdate, elapsedRender;
    ChangePerSecond ups, fps;
};
}   // namespace MapleLeaf