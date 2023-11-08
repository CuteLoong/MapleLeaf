#include "LogicalDevice.hpp"
#include "Graphics.hpp"
#include "Log.hpp"
#include "PhysicalDevice.hpp"
#include "vulkan/vulkan_core.h"

namespace MapleLeaf {
const std::vector<const char*> LogicalDevice::DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                                  VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                                                                  VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
                                                                  VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
                                                                  VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME};   // VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME

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
    std::vector<float>                   queuePriorities(3, 0.0f);   // This is the special case where all queues are from the same queue group

    if (supportedQueues & VK_QUEUE_GRAPHICS_BIT) {
        VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
        graphicsQueueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphicsQueueCreateInfo.queueFamilyIndex        = graphicsFamily.value();
        graphicsQueueCreateInfo.queueCount              = queuePriorities.size();
        graphicsQueueCreateInfo.pQueuePriorities        = queuePriorities.data();
        queueCreateInfos.emplace_back(graphicsQueueCreateInfo);
    }
    else {
        graphicsFamily = 0;   // VK_NULL_HANDLE;
    }

    if (supportedQueues & VK_QUEUE_COMPUTE_BIT && computeFamily != graphicsFamily) {
        VkDeviceQueueCreateInfo computeQueueCreateInfo = {};
        computeQueueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        computeQueueCreateInfo.queueFamilyIndex        = computeFamily.value();
        computeQueueCreateInfo.queueCount              = queuePriorities.size();
        computeQueueCreateInfo.pQueuePriorities        = queuePriorities.data();
        queueCreateInfos.emplace_back(computeQueueCreateInfo);
    }
    else {
        computeFamily = graphicsFamily;
    }

    if (supportedQueues & VK_QUEUE_TRANSFER_BIT && transferFamily != graphicsFamily && transferFamily != computeFamily) {
        VkDeviceQueueCreateInfo transferQueueCreateInfo = {};
        transferQueueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        transferQueueCreateInfo.queueFamilyIndex        = transferFamily.value();
        transferQueueCreateInfo.queueCount              = queuePriorities.size();
        transferQueueCreateInfo.pQueuePriorities        = queuePriorities.data();
        queueCreateInfos.emplace_back(transferQueueCreateInfo);
    }
    else {
        transferFamily = graphicsFamily;
    }

    auto physicalDeviceFeatures = physicalDevice.GetFeatures();
    bindlessMaxDescriptorsCount = physicalDevice.GetProperties().limits.maxDescriptorSetStorageBuffers / 3.0f;

    VkPhysicalDeviceFeatures2 extensionFeatures      = {};
    void*                     deviceCreatepNextChain = nullptr;

    if (physicalDeviceFeatures.sampleRateShading) enabledFeatures.sampleRateShading = VK_TRUE;
    if (physicalDeviceFeatures.multiDrawIndirect) enabledFeatures.multiDrawIndirect = VK_TRUE;

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

    if (physicalDeviceFeatures.multiViewport)
        enabledFeatures.multiViewport = VK_TRUE;
    else
        Log::Warning("Selected GPU does not support multiple viewport!\n");

    if (physicalDeviceFeatures.fragmentStoresAndAtomics)
        enabledFeatures.fragmentStoresAndAtomics = VK_TRUE;
    else
        Log::Warning("Selected GPU does not support fragmentStoresAndAtomics!\n");

    if (physicalDeviceFeatures.independentBlend)
        enabledFeatures.independentBlend = VK_TRUE;
    else
        Log::Warning("Selected GPU does not support independentBlend!\n");

    // add bindless feature
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
    indexingFeatures.sType                                         = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
    indexingFeatures.runtimeDescriptorArray                        = VK_TRUE;
    indexingFeatures.descriptorBindingVariableDescriptorCount      = VK_TRUE;
    indexingFeatures.shaderSampledImageArrayNonUniformIndexing     = VK_TRUE;
    indexingFeatures.descriptorBindingUpdateUnusedWhilePending     = VK_TRUE;
    indexingFeatures.descriptorBindingPartiallyBound               = VK_TRUE;
    indexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingSampledImageUpdateAfterBind  = VK_TRUE;
    indexingFeatures.pNext                                         = nullptr;

    VkPhysicalDeviceMaintenance4Features maintenance4Features = {};
    maintenance4Features.sType                                = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
    maintenance4Features.maintenance4                         = VK_TRUE;
    maintenance4Features.pNext                                = &indexingFeatures;

    deviceCreatepNextChain     = &maintenance4Features;
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
    vkGetDeviceQueue(logicalDevice, computeFamily.value(), 1, &computeQueue);
    vkGetDeviceQueue(logicalDevice, transferFamily.value(), 2, &transferQueue);
}
}   // namespace MapleLeaf