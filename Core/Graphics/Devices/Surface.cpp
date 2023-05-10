#include "Surface.hpp"
#include "Graphics.hpp"
#include "Instance.hpp"
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "vulkan/vulkan_core.h"

namespace MapleLeaf {
Surface::Surface(const Instance& instance, const PhysicalDevice& physicalDevice, const LogicalDevice& logicalDevice, const Window& window)
    : instance(instance)
    , physicalDevice(physicalDevice)
    , logicalDevice(logicalDevice)
    , window(window)
{
    Graphics::CheckVk(window.CreateSurface(instance, nullptr, &surface));
    Graphics::CheckVk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities));

    uint32_t surfaceFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data());

    if (surfaceFormatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
        format.format     = VK_FORMAT_B8G8R8A8_UNORM;
        format.colorSpace = surfaceFormats[0].colorSpace;
    }
    else {
        bool found_B8G8R8A8_UNORM = false;

        for (auto& surfaceFormat : surfaceFormats) {
            if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM) {
                format.format        = surfaceFormat.format;
                format.colorSpace    = surfaceFormat.colorSpace;
                found_B8G8R8A8_UNORM = true;
                break;
            }
        }

        if (!found_B8G8R8A8_UNORM) {
            format.format     = surfaceFormats[0].format;
            format.colorSpace = surfaceFormats[0].colorSpace;
        }
    }

    VkBool32 presentSupport;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, logicalDevice.GetPresentFamily(), surface, &presentSupport);

    if (!presentSupport) throw std::runtime_error("Present queue family does not have presentation support");
}

Surface::~Surface()
{
    vkDestroySurfaceKHR(instance, surface, nullptr);
}
}   // namespace MapleLeaf