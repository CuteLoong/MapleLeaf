#pragma once

#include "KeyEnums.hpp"
#include "NonCopyable.hpp"
#include "boost/signals2.hpp"
#include "glm/glm.hpp"
#include "volk.h"
#include <string>
#include <vector>

struct GLFWwindow;

namespace MapleLeaf {
class Window : public NonCopyable
{
public:
    Window(std::size_t id);
    ~Window();

    void Update();

    const glm::uvec2& GetSize(bool checkFullscreen = true) const;
    const glm::uvec2 GetStereoSize(bool checkFullscreen = true) const;

    void              SetSize(const glm::uvec2& size);
    float             GetAspectRatio() const;
    float             GetStereoAspectRatio() const;


    const std::string& GetTitle();
    void               SetTitle(const std::string& title);

    bool IsResizable() const;
    void SetResizable(bool resizable);

    bool IsFullscreen() const;
    void SetFullscreen(bool fullscreen);

    bool IsClosed() const;
    bool IsFocused() const;
    bool IsIconified() const;

    bool IsWindowSelected() const;
    bool IsCursorHidden() const;
    void SetCursorHidden(bool hidden);

    const glm::vec2& GetMousePosition() const;
    void             SetMousePosition(const glm::vec2& mousePosition);
    const glm::vec2& GetMousePositionDelta() const;

    const glm::vec2& GetMouseScroll() const;
    void             SetMouseScroll(const glm::vec2& scroll);
    const glm::vec2& GetMouseScrollDelta() const;

    GLFWwindow* GetWindow() const;

    VkResult CreateSurface(const VkInstance& instance, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface) const;

    boost::signals2::signal<void(glm::vec2)>&                          OnMousePosition() { return onMousePosition; }
    boost::signals2::signal<void(MouseButton, InputAction, InputMod)>& OnMouseButton() { return onMouseButton; }
    boost::signals2::signal<void(Key, InputAction, InputMod)>& OnKey() { return onKey; }

private:
    std::size_t windowId;
    GLFWwindow* window = nullptr;

    glm::uvec2 size;
    glm::uvec2 fullscreenSize;

    std::string title;
    bool        resizable  = false;
    bool        fullscreen = false;

    bool closed    = false;
    bool focused   = false;
    bool iconified = false;

    bool windowSeleted = false;
    bool cursorHidden  = false;

    glm::vec2 mouseLastPosition;
    glm::vec2 mousePosition;
    glm::vec2 mousePositionDelta;

    glm::vec2 mouseLastScroll;
    glm::vec2 mouseScroll;
    glm::vec2 mouseScrollDelta;

    boost::signals2::signal<void(glm::vec2)>                          onMousePosition;
    boost::signals2::signal<void(MouseButton, InputAction, InputMod)> onMouseButton;
    boost::signals2::signal<void(Key, InputAction, InputMod)> onKey;

    friend void CallbackWindowClose(GLFWwindow* glfwWindow);
    friend void CallbackWindowSize(GLFWwindow* glfwWindow, int32_t width, int32_t height);
    friend void CallbackFramebufferSize(GLFWwindow* glfwWindow, int32_t width, int32_t height);
    friend void CallbackWindowFocus(GLFWwindow* glfwWindow, int32_t focused);
    friend void CallbackWindowIconify(GLFWwindow* glfwWindow, int32_t iconified);
    friend void CallbackCursorPos(GLFWwindow* glfwWindow, double xpos, double ypos);
    friend void CallbackMouseButton(GLFWwindow* glfwWindow, int32_t button, int32_t action, int32_t mods);
    friend void CallbackKey(GLFWwindow* glfwWindow, int32_t key, int32_t scancode, int32_t action, int32_t mods);
};
}   // namespace MapleLeaf