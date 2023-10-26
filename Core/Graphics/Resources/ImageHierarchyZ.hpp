#pragma once

#include "Image.hpp"

namespace MapleLeaf {
class ImageHierarchyZ : public Image
{
public:
    explicit ImageHierarchyZ(const glm::uvec2& extent, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

    void AddHierarchicalDepth(const CommandBuffer& commandBuffer, const VkImage& depth, const VkExtent3D& depthExtent, VkFormat depthFormat,
                              VkImageLayout depthLayout, uint32_t hizMipLevel, uint32_t hizArrayLayer) const;

    static uint32_t GetMipLevels(const glm::uvec2& extent);

    VkExtent2D GetExtentByMipLevel(uint32_t mipLevel);

private:
};
}   // namespace MapleLeaf