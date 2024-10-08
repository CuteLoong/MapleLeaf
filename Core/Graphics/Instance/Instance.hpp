#pragma once

#include "volk.h"
#include <vector>

namespace MapleLeaf {
class Instance
{
    friend class Graphics;

public:
    static const std::vector<const char*> ValidationLayers;

    friend VKAPI_ATTR VkBool32 VKAPI_CALL CallbackDebug(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    Instance();
    ~Instance();

    static VkResult FvkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    static void FvkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator);

    static void FvkCmdPushDescriptorSetKHR(VkDevice device, VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                           VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                           const VkWriteDescriptorSet* pDescriptorWrites);

    static uint32_t FindMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties* deviceMemoryProperties,
                                        const VkMemoryRequirements* memoryRequirements, VkMemoryPropertyFlags requiredProperties);

    operator const VkInstance&() const { return instance; }

    bool GetEnableValidationLayers() const { return enableValidationLayers; }

private:
    VkInstance instance               = VK_NULL_HANDLE;
    bool       enableValidationLayers = false;

    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    bool                     CheckValidationLayerSupport() const;
    std::vector<const char*> GetExtensions() const;
    void                     CreateInstance();
    void                     CreateDebugMessenger();

    static void LogVulkanLayers(const std::vector<VkLayerProperties>& layerProperties);
};
}   // namespace MapleLeaf