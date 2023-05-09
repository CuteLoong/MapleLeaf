#include "Devices.hpp"
#include "Engine.hpp"
#include "Graphics.hpp"

#include "GLFW/glfw3.h"
#include <iostream>
#include <memory>

int main(int argc, char** argv)
{
    using namespace MapleLeaf;
    // Engine engine(argv[0]);
    auto engine = std::make_unique<Engine>(argv[0]);
    // auto    device = Devices::Get();
    // Window* window = device->CreateWindow();

    // while (!window->IsClosed()) {
    //     glfwPollEvents();
    // }

    return 0;
}