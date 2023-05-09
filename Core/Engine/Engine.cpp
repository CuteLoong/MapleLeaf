#include "Engine.hpp"
#include "Log.hpp"

namespace MapleLeaf {
Engine* Engine::Instance = nullptr;

Engine::Engine(std::string argv0, ModuleFilter&& moduleFilter)
    : argv0(std::move(argv0))
    , engineVersion(1, 0, 0)
{
    Instance = this;
    Log::OpenLog(Time::GetDateTime("Logs/%Y%m%d%H%M%S.txt"));
    for (auto it = Module::Registry().begin(); it != Module::Registry().end(); ++it) CreateModule(it, moduleFilter);
}

Engine::~Engine()
{
    for (auto it = modules.rbegin(); it != modules.rend(); ++it) DestroyModule(it->first);

    Log::CloseLog();
}

void Engine::CreateModule(Module::TRegistryMap::const_iterator it, const ModuleFilter& filter)
{
    if (modules.find(it->first) != modules.end()) return;
    if (!filter.Check(it->first)) return;

    // TODO: Prevent circular dependencies.
    for (auto requireId : it->second.requiremets) CreateModule(Module::Registry().find(requireId), filter);

    auto&& module      = it->second.create();
    modules[it->first] = std::move(module);
    moduleStages[it->second.stage].emplace_back(it->first);
}
void Engine::DestroyModule(TypeId id)
{
    if (!modules[id]) return;

    // Destroy all module dependencies first.
    for (const auto& [registrarId, registrar] : Module::Registry()) {
        if (std::find(registrar.requiremets.begin(), registrar.requiremets.end(), id) != registrar.requiremets.end()) DestroyModule(registrarId);
    }
    modules[id] = nullptr;
}

void Engine::UpdateStage(Module::Stage stage)
{
    for (auto& moduleId : moduleStages[stage]) modules[moduleId]->Update();
}
}   // namespace MapleLeaf