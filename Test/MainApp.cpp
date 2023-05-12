#include "MainApp.hpp"
#include "Graphics.hpp"
#include "Log.hpp"
#include "MainRenderer.hpp"
#include <memory>


int main(int argc, char** argv)
{
    using namespace Test;
    auto engine = std::make_unique<Engine>(argv[0]);
    engine->SetApp(std::make_unique<MainApp>());

    auto exitCode = engine->Run();
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
}

void MainApp::Update()
{
    Graphics::Get()->Update();
}
}   // namespace Test