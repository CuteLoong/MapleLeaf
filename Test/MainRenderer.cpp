#include "MainRenderer.hpp"

#include "Graphics.hpp"
#include <iostream>

namespace Test {
MainRenderer::MainRenderer()
{
    std::cout << "Create Main Renderer" << std::endl;
}

void MainRenderer::Start()
{
    std::cout << "Main Renderer Start" << std::endl;
}

void MainRenderer::Update()
{
    std::cout << "Main Renderer Update" << std::endl;
}
}   // namespace Test