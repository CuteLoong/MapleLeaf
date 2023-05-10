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
    auto    engine = std::make_unique<Engine>(argv[0]);
    Window* window = Devices::Get()->GetWindow();

    while (!window->IsClosed()) {
        glfwPollEvents();
    }

    return 0;
}