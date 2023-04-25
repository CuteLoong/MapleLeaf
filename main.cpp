#include "Engine.hpp"
#include "Window.hpp"
#include <iostream>
#include <memory>

int main(int argc, char** argv)
{
    using namespace MapleLeaf;
    // Engine engine(argv[0]);
    auto   engine = std::make_unique<Engine>(argv[0]);
    Window window(0);

    while (window.IsClosed()) {}

    return 0;
}