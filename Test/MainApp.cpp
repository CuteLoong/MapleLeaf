#include "MainApp.hpp"
#include "Devices.hpp"
#include "Engine.hpp"
#include "Graphics.hpp"
#include "Log.hpp"
#include "MainRenderer.hpp"
#include "SceneBuilder.hpp"
#include "Scenes.hpp"

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
    : App("Test Triangle1", {1, 0, 0})
{
    // Registers file search paths.
    Log::Out("Working Directory: ", std::filesystem::current_path(), '\n');
}

MainApp::~MainApp() {}

void MainApp::Start()
{
    Devices::Get()->GetWindow()->SetTitle("Test Title");
    Graphics::Get()->SetRenderer(std::make_unique<MainRenderer>());

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