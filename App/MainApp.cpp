#include "MainApp.hpp"
#include "Engine.hpp"
#include "GPURenderer.hpp"
#include "Log.hpp"
#include "MainRenderer.hpp"
#include "RawSSRRenderer.hpp"
#include "RayTracingRenderer.hpp"
#include "SCSSRRenderer.hpp"
#include "SceneBuilder.hpp"
#include "Scenes.hpp"
#include "StereoRayTracingRenderer.hpp"
#include "StereoRenderer.hpp"
#include "WarpRenderer.hpp"


#include "config.h"

int main(int argc, char** argv)
{
    using namespace Test;
    auto engine = std::make_unique<Engine>(argv[0]);
    engine->SetApp(std::make_unique<MainApp>());

    auto exitCode = engine->Run();
    engine        = nullptr;

    std::cout << "Press enter to continue...";
    std::cin.get();
    return exitCode;
}

namespace Test {
MainApp::MainApp()
    : App("MapleLeaf", {CONFIG_VERSION_MAJOR, CONFIG_VERSION_MINOR, CONFIG_VERSION_ALTER})
{
    // Registers file search paths.
    Log::Out("Working Directory: ", std::filesystem::current_path(), '\n');
    Files::Get()->AddSearchPath("Resources");
    Files::Get()->AddSearchPath("Resources/Shader");
    Files::Get()->AddSearchPath("Resources/Skybox");
}

MainApp::~MainApp() {}

void MainApp::Start()
{
    Devices::Get()->GetWindow()->SetTitle("MapleLeaf");

#ifdef MAPLELEAF_RAY_TRACING
    // Graphics::Get()->SetRenderer(std::make_unique<RayTracingRenderer>());
    Graphics::Get()->SetRenderer(std::make_unique<StereoRayTracingRenderer>());
#else
    // Graphics::Get()->SetRenderer(std::make_unique<MainRenderer>());
    // Graphics::Get()->SetRenderer(std::make_unique<GPURenderer>());
    // Graphics::Get()->SetRenderer(std::make_unique<StereoRenderer>());
    // Graphics::Get()->SetRenderer(std::make_unique<RawSSRRenderer>());
    Graphics::Get()->SetRenderer(std::make_unique<WarpRenderer>());
    // Graphics::Get()->SetRenderer(std::make_unique<SCSSRRenderer>());
#endif

    std::unique_ptr<SceneBuilder> scene = std::make_unique<SceneBuilder>();
    Scenes::Get()->SetScene(std::move(scene));
}

void MainApp::Update()
{
    if (Devices::Get()->GetWindow()->IsClosed()) {
        Engine::Get()->RequestClose();
    }
}
}   // namespace Test