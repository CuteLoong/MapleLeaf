#pragma once

#include "volk.h"
#include <optional>
#include <vector>

namespace MapleLeaf {
class Instance;
class PhysicalDevice;

class LogicalDevice
{
    friend class Graphics;

public:
    LogicalDevice(const Instance& instance, const PhysicalDevice& physicalDevice);
    ~LogicalDevice();

    operator const VkDevice&() const { return logicalDevice; }

    const VkDevice&                 GetLogicalDevice() const { return logicalDevice; }
    const VkPhysicalDeviceFeatures& GetEnabledFeatures() const { return enabledFeatures; }
    const VkQueue&                  GetGraphicsQueue() const { return graphicsQueue; }
    const VkQueue&                  GetPresentQueue() const { return presentQueue; }
    const VkQueue&                  GetComputeQueue() const { return computeQueue; }
    const VkQueue&                  GetTransferQueue() const { return transferQueue; }
    uint32_t                        GetGraphicsFamily() const { return graphicsFamily.value(); }
    uint32_t                        GetPresentFamily() const { return presentFamily.value(); }
    uint32_t                        GetComputeFamily() const { return computeFamily.value(); }
    uint32_t                        GetTransferFamily() const { return transferFamily.value(); }

    static const std::vector<const char*> DeviceExtensions;

private:
    const Instance&       instance;
    const PhysicalDevice& physicalDevice;

    VkDevice                 logicalDevice   = VK_NULL_HANDLE;
    VkPhysicalDeviceFeatures enabledFeatures = {};

    VkQueueFlags            supportedQueues = {};
    std::optional<uint32_t> graphicsFamily  = 0;
    std::optional<uint32_t> presentFamily   = 0;
    std::optional<uint32_t> computeFamily   = 0;
    std::optional<uint32_t> transferFamily  = 0;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue  = VK_NULL_HANDLE;
    VkQueue computeQueue  = VK_NULL_HANDLE;
    VkQueue transferQueue = VK_NULL_HANDLE;

    void CreateQueueIndices();
    void CreateLogicalDevice();
};
}   // namespace MapleLeaf