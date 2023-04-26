#include "Devices.hpp"

#include "GLFW/glfw3.h"
#include <iostream>
#include <memory>


namespace MapleLeaf {
Devices::Devices()
{
    if (glfwInit() == GLFW_FALSE) throw std::runtime_error("GLFW failed to initialize");
    if (glfwVulkanSupported() == GLFW_FALSE) throw std::runtime_error("GLFW failed to find Vulkan support");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_STEREO, GLFW_FALSE);
    // std::cout << "go here" << std::endl;
}

Devices::~Devices()
{
    glfwTerminate();
}

void Devices::Update()
{
    glfwPollEvents();
    window->Update();
}

Window* Devices::CreateWindow()
{
    window = std::make_unique<Window>(0);
    return window.get();
}
const Window* Devices::GetWindow() const
{
    return window.get();
}
Window* Devices::GetWindow()
{
    return window.get();
}

std::pair<const char**, uint32_t> Devices::GetInstanceExtensions() const
{
    uint32_t glfwExtensionCount;
    auto     glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    return std::make_pair(glfwExtensions, glfwExtensionCount);
}
}   // namespace MapleLeaf