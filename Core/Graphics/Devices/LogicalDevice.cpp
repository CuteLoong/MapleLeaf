#include "LogicalDevice.hpp"
#include "Graphics.hpp"
#include "Log.hpp"
#include "PhysicalDevice.hpp"
#include "vulkan/vulkan_core.h"


namespace MapleLeaf {
const std::vector<const char*> LogicalDevice::DeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};   // VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME

LogicalDevice::LogicalDevice(const Instance& instance, const PhysicalDevice& physicalDevice)
    : instance(instance)
    , physicalDevice(physicalDevice)
{
    CreateQueueIndices();
    CreateLogicalDevice();
}

LogicalDevice::~LogicalDevice()
{
    Graphics::CheckVk(vkDeviceWaitIdle(logicalDevice));

    vkDestroyDevice(logicalDevice, nullptr);
}

void LogicalDevice::CreateQueueIndices()
{
    uint32_t deviceQueueFamilyPropertyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &deviceQueueFamilyPropertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> deviceQueueFamilyProperties(deviceQueueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &deviceQueueFamilyPropertyCount, deviceQueueFamilyProperties.data());

    for (uint32_t i = 0; i < deviceQueueFamilyPropertyCount; i++) {
        if (deviceQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            this->graphicsFamily = i;
            this->presentFamily  = i;
            supportedQueues |= VK_QUEUE_GRAPHICS_BIT;
        }

        if (deviceQueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            this->computeFamily = i;
            supportedQueues |= VK_QUEUE_COMPUTE_BIT;
        }

        if (deviceQueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            this->transferFamily = i;
            supportedQueues |= VK_QUEUE_TRANSFER_BIT;
        }

        if (this->graphicsFamily && this->presentFamily && this->computeFamily && this->transferFamily) {
            break;
        }
    }

    if (!this->graphicsFamily) throw std::runtime_error("Failed to find queue family supporting VK_QUEUE_GRAPHICS_BIT");
}

void LogicalDevice::CreateLogicalDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float                                queuePriorities[1] = {0.0f};

    if (supportedQueues & VK_QUEUE_GRAPHICS_BIT) {
        VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
        graphicsQueueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphicsQueueCreateInfo.queueFamilyIndex        = graphicsFamily.value();
        graphicsQueueCreateInfo.queueCount              = 1;
        graphicsQueueCreateInfo.pQueuePriorities        = queuePriorities;
        queueCreateInfos.emplace_back(graphicsQueueCreateInfo);
    }
    else {
        graphicsFamily = 0;   // VK_NULL_HANDLE;
    }

    if (supportedQueues & VK_QUEUE_COMPUTE_BIT && computeFamily != graphicsFamily) {
        VkDeviceQueueCreateInfo computeQueueCreateInfo = {};
        computeQueueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        computeQueueCreateInfo.queueFamilyIndex        = computeFamily.value();
        computeQueueCreateInfo.queueCount              = 1;
        computeQueueCreateInfo.pQueuePriorities        = queuePriorities;
        queueCreateInfos.emplace_back(computeQueueCreateInfo);
    }
    else {
        computeFamily = graphicsFamily;
    }

    if (supportedQueues & VK_QUEUE_TRANSFER_BIT && transferFamily != graphicsFamily && transferFamily != computeFamily) {
        VkDeviceQueueCreateInfo transferQueueCreateInfo = {};
        transferQueueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        transferQueueCreateInfo.queueFamilyIndex        = transferFamily.value();
        transferQueueCreateInfo.queueCount              = 1;
        transferQueueCreateInfo.pQueuePriorities        = queuePriorities;
        queueCreateInfos.emplace_back(transferQueueCreateInfo);
    }
    else {
        transferFamily = graphicsFamily;
    }

    auto physicalDeviceFeatures = physicalDevice.GetFeatures();

    VkPhysicalDeviceFeatures2 extensionFeatures      = {};
    void*                     deviceCreatepNextChain = nullptr;

    if (physicalDeviceFeatures.sampleRateShading) enabledFeatures.sampleRateShading = VK_TRUE;

    if (physicalDeviceFeatures.geometryShader)
        enabledFeatures.geometryShader = VK_TRUE;
    else
        Log::Warning("Selected GPU does not support geometry shaders!\n");

    if (physicalDeviceFeatures.tessellationShader)
        enabledFeatures.tessellationShader = VK_TRUE;
    else
        Log::Warning("Selected GPU does not support tessellation shaders!\n");

    if (physicalDeviceFeatures.samplerAnisotropy)
        enabledFeatures.samplerAnisotropy = VK_TRUE;
    else
        Log::Warning("Selected GPU does not support sampler anisotropy!\n");

    // add bindless feature
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
    indexingFeatures.sType                                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
    indexingFeatures.runtimeDescriptorArray                    = VK_TRUE;
    indexingFeatures.descriptorBindingVariableDescriptorCount  = VK_TRUE;
    indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    indexingFeatures.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
    indexingFeatures.descriptorBindingPartiallyBound           = VK_TRUE;
    indexingFeatures.pNext                                     = nullptr;

    deviceCreatepNextChain     = &indexingFeatures;
    extensionFeatures.sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    extensionFeatures.features = enabledFeatures;
    extensionFeatures.pNext    = deviceCreatepNextChain;

    VkDeviceCreateInfo deviceCreateInfo   = {};
    deviceCreateInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos    = queueCreateInfos.data();
    if (instance.GetEnableValidationLayers()) {
        deviceCreateInfo.enabledLayerCount   = static_cast<uint32_t>(Instance::ValidationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = Instance::ValidationLayers.data();
    }
    deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(DeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();
    deviceCreateInfo.pNext                   = &extensionFeatures;
    // deviceCreateInfo.pEnabledFeatures        = &enabledFeatures;
    Graphics::CheckVk(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice));

    volkLoadDevice(logicalDevice);

    vkGetDeviceQueue(logicalDevice, graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, presentFamily.value(), 0, &presentQueue);
    vkGetDeviceQueue(logicalDevice, computeFamily.value(), 0, &computeQueue);
    vkGetDeviceQueue(logicalDevice, transferFamily.value(), 0, &transferQueue);
}
}   // namespace MapleLeaf