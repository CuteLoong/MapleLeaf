#pragma once

#include "CommandBuffer.hpp"
#include "volk.h"
#include <memory>
#include <vector>

namespace MapleLeaf {
class Instance;
class LogicalDevice;
class PhysicalDevice;
class Window;

class Surface
{
    friend class Graphics;

public:
    Surface(const Instance& instance, const PhysicalDevice& physicalDevice, const LogicalDevice& logicalDevice, const Window& window);
    ~Surface();

    operator const VkSurfaceKHR&() const { return surface; }

    const VkSurfaceKHR&             GetSurface() const { return surface; }
    const VkSurfaceCapabilitiesKHR& GetCapabilities() const { return capabilities; }
    const VkSurfaceFormatKHR&       GetFormat() const { return format; }

    void SetPresentSemaphoreSize(std::size_t size) { this->presentCompletes.resize(size); }
    void SetRenderSemaphoreSize(std::size_t size) { this->renderCompletes.resize(size); }
    void SetFilghtFenceSize(std::size_t size) { this->flightFences.resize(size); }
    void SetCommandBufferSize(std::size_t size) { this->commandBuffers.resize(size); }

    void SetCurrentFrameIndex(std::size_t index) { this->currentFrameIndex = index; }
    void SetFramebufferResized() { this->framebufferResized = true; }

    std::vector<VkSemaphore>&                    GetPresentSemaphores() { return presentCompletes; }
    std::vector<VkSemaphore>&                    GetRenderSemaphores() { return renderCompletes; }
    std::vector<VkFence>&                        GetFilghtFences() { return flightFences; }
    std::vector<std::unique_ptr<CommandBuffer>>& GetCommandBuffers() { return commandBuffers; }

    const std::size_t GetCurrentFrameIndex() const { return currentFrameIndex; }
    const bool        GetFramebufferResized() const { return framebufferResized; }

private:
    const Instance&       instance;
    const PhysicalDevice& physicalDevice;
    const LogicalDevice&  logicalDevice;
    const Window&         window;

    VkSurfaceKHR             surface      = VK_NULL_HANDLE;
    VkSurfaceCapabilitiesKHR capabilities = {};
    VkSurfaceFormatKHR       format       = {};

    // surfaceBuffer init in runtime(Graphics)
    std::vector<VkSemaphore>                    presentCompletes;
    std::vector<VkSemaphore>                    renderCompletes;
    std::vector<VkFence>                        flightFences;
    std::size_t                                 currentFrameIndex  = 0;
    bool                                        framebufferResized = false;
    std::vector<std::unique_ptr<CommandBuffer>> commandBuffers;
};
}   // namespace MapleLeaf