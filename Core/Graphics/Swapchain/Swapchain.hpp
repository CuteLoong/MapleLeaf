#pragma once

#include "volk.h"
#include <vector>

namespace MapleLeaf {
class PhysicalDevice;
class Surface;
class LogicalDevice;

class Swapchain
{
public:
    Swapchain(const PhysicalDevice& physicalDevice, const Surface& surface, const LogicalDevice& logicalDevice, const VkExtent2D& extent,
              const Swapchain* oldSwapchain = nullptr);
    ~Swapchain();

    VkResult AcquireNextImage(const VkSemaphore& presentCompleteSemaphore = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE);
    VkResult QueuePresent(const VkQueue& presentQueue, const VkSemaphore& waitSemaphore = VK_NULL_HANDLE);

    bool IsSameExtent(const VkExtent2D& extent2D) { return extent.width == extent2D.width && extent.height == extent2D.height; }

    operator const VkSwapchainKHR&() const { return swapchain; }

    const VkExtent2D&               GetExtent() const { return extent; }
    uint32_t                        GetImageCount() const { return imageCount; }
    VkSurfaceTransformFlagsKHR      GetPreTransform() const { return preTransform; }
    VkCompositeAlphaFlagBitsKHR     GetCompositeAlpha() const { return compositeAlpha; }
    const std::vector<VkImage>&     GetImages() const { return images; }
    const VkImage&                  GetActiveImage() const { return images[activeImageIndex]; }
    const std::vector<VkImageView>& GetImageViews() const { return imageViews; }
    const VkSwapchainKHR&           GetSwapchain() const { return swapchain; }
    uint32_t                        GetActiveImageIndex() const { return activeImageIndex; }

private:
    const PhysicalDevice& physicalDevice;
    const Surface&        surface;
    const LogicalDevice&  logicalDevice;

    VkExtent2D                  extent;
    VkPresentModeKHR            presentMode;
    VkSurfaceTransformFlagsKHR  preTransform;
    VkCompositeAlphaFlagBitsKHR compositeAlpha;

    uint32_t                 imageCount = 0;
    std::vector<VkImage>     images;
    std::vector<VkImageView> imageViews;
    VkSwapchainKHR           swapchain = VK_NULL_HANDLE;

    VkFence  fenceImage = VK_NULL_HANDLE;
    uint32_t activeImageIndex;
};
}   // namespace MapleLeaf