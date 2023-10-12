#include "Window.hpp"

#include "Engine.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <stdexcept>

namespace MapleLeaf {
void CallbackWindowClose(GLFWwindow* glfwWindow)
{
    auto window    = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->closed = true;
}

void CallbackWindowSize(GLFWwindow* glfwWindow, int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0) return;
    auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

    if (window->fullscreen) {
        window->fullscreenSize = {width, height};
    }
    else {
        window->size = {width, height};
    }
}

void CallbackFramebufferSize(GLFWwindow* glfwWindow, int32_t width, int32_t height)
{
    auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    if (window->fullscreen)
        window->fullscreenSize = {width, height};
    else
        window->size = {width, height};
    // TODO: set framebuffer to resized
}

void CallbackWindowFocus(GLFWwindow* glfwWindow, int32_t focused)
{
    auto window     = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->focused = static_cast<bool>(focused);
}

void CallbackWindowIconify(GLFWwindow* glfwWindow, int32_t iconified)
{
    auto window       = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->iconified = static_cast<bool>(iconified);
}

void CallbackMouseButton(GLFWwindow* glfwWindow, int32_t button, int32_t action, int32_t mods)
{
    auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->onMouseButton(static_cast<MouseButton>(button), static_cast<InputAction>(action), static_cast<InputMod>(mods));
}

void CallbackCursorPos(GLFWwindow* glfwWindow, double xpos, double ypos)
{
    auto window           = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->mousePosition = {xpos, ypos};
    window->onMousePosition(window->mousePosition);
}

void CallbackKey(GLFWwindow* glfwWindow, int32_t key, int32_t scancode, int32_t action, int32_t mods)
{
    auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->onKey(static_cast<Key>(key), static_cast<InputAction>(action), static_cast<InputMod>(mods));
}

Window::Window(std::size_t id)
    : windowId(id)
    , size(1920, 1080)
    , title("MapleLeaf Window")
    , resizable(true)
    , focused(true)
{
    window = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        throw std::runtime_error("GLFW failed to create the window");
    }

    // Sets the user pointer.
    glfwSetWindowUserPointer(window, this);

    glfwSetWindowAttrib(window, GLFW_RESIZABLE, resizable);
    if (fullscreen) SetFullscreen(true);

    glfwShowWindow(window);

    glfwSetWindowCloseCallback(window, CallbackWindowClose);
    glfwSetWindowFocusCallback(window, CallbackWindowFocus);
    glfwSetWindowIconifyCallback(window, CallbackWindowIconify);
    glfwSetWindowSizeCallback(window, CallbackWindowSize);
    glfwSetFramebufferSizeCallback(window, CallbackFramebufferSize);
    glfwSetCursorPosCallback(window, CallbackCursorPos);
    glfwSetMouseButtonCallback(window, CallbackMouseButton);
    glfwSetKeyCallback(window, CallbackKey);
}
Window::~Window()
{
    glfwDestroyWindow(window);
    closed = true;
}

void Window::Update()
{
    auto delta = Engine::Get()->GetDelta().AsSeconds();

    // Updates the position delta.
    mousePositionDelta = delta * (mouseLastPosition - mousePosition);
    mouseLastPosition  = mousePosition;

    // Updates the scroll delta.
    mouseScrollDelta = delta * (mouseLastScroll - mouseScroll);
    mouseLastScroll  = mouseScroll;
}

const glm::uvec2& Window::GetSize(bool checkFullscreen) const
{
    return (fullscreen && checkFullscreen) ? fullscreenSize : size;
}

const glm::uvec2 Window::GetStereoSize(bool checkFullscreen) const
{
    return (fullscreen && checkFullscreen) ? glm::uvec2(fullscreenSize.x / 2.0f, fullscreenSize.y) : glm::uvec2(size.x / 2.0f, size.y);
}

void Window::SetSize(const glm::uvec2& size)
{
    if (size.x != -1) this->size.x = size.x;
    if (size.y != -1) this->size.y = size.y;
    glfwSetWindowSize(window, size.x, size.y);
}

float Window::GetAspectRatio() const
{
    return static_cast<float>(GetSize().x) / static_cast<float>(GetSize().y);
}

float Window::GetStereoAspectRatio() const
{
    return static_cast<float>(GetSize().x / 2.0f) / static_cast<float>(GetSize().y);
}

const std::string& Window::GetTitle()
{
    return title;
}
void Window::SetTitle(const std::string& title)
{
    this->title = title;
    glfwSetWindowTitle(window, title.c_str());
}

bool Window::IsResizable() const
{
    return resizable;
}
void Window::SetResizable(bool resizable)
{
    this->resizable = resizable;
    glfwSetWindowAttrib(window, GLFW_RESIZABLE, resizable);
}

bool Window::IsFullscreen() const
{
    return fullscreen;
}
void Window::SetFullscreen(bool fullscreen)
{
    this->fullscreen = fullscreen;
}

bool Window::IsClosed() const
{
    return closed;
}
bool Window::IsFocused() const
{
    return focused;
}
bool Window::IsIconified() const
{
    return iconified;
}

bool Window::IsWindowSelected() const
{
    return windowSeleted;
}
bool Window::IsCursorHidden() const
{
    return cursorHidden;
}
void Window::SetCursorHidden(bool hidden)
{
    if (cursorHidden != hidden) {
        glfwSetInputMode(window, GLFW_CURSOR, hidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

        if (!hidden && cursorHidden) SetMousePosition(mousePosition);
    }

    cursorHidden = hidden;
}

const glm::vec2& Window::GetMousePosition() const
{
    return mousePosition;
}
void Window::SetMousePosition(const glm::vec2& mousePosition)
{
    this->mouseLastPosition = mousePosition;
    this->mousePosition     = mousePosition;
    glfwSetCursorPos(window, mousePosition.x, mousePosition.y);
}
const glm::vec2& Window::GetMousePositionDelta() const
{
    return mousePositionDelta;
}

const glm::vec2& Window::GetMouseScroll() const
{
    return mouseScroll;
}
void Window::SetMouseScroll(const glm::vec2& scroll)
{
    this->mouseLastScroll = scroll;
    this->mouseScroll     = scroll;
}
const glm::vec2& Window::GetMouseScrollDelta() const
{
    return mouseScrollDelta;
}

GLFWwindow* Window::GetWindow() const
{
    return window;
}

VkResult Window::CreateSurface(const VkInstance& instance, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface) const
{
    return glfwCreateWindowSurface(instance, window, allocator, surface);
}
}   // namespace MapleLeaf