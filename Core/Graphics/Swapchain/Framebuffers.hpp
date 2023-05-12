#pragma once

#include "Image2d.hpp"
#include "NonCopyable.hpp"
#include "Swapchain.hpp"
#include "glm/glm.hpp"


namespace MapleLeaf {
class LogicalDevice;
class ImageDepth;
class RenderStage;
class Renderpass;

class Framebuffers : NonCopyable
{
public:
    Framebuffers(const LogicalDevice& logicalDevice, const Swapchain& swapchain, const RenderStage& renderStage, const Renderpass& renderPass,
                 const ImageDepth& depthStencil, const glm::uvec2& extent, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    ~Framebuffers();

    Image2d* GetAttachment(uint32_t index) const { return imageAttachments[index].get(); }

    const std::vector<std::unique_ptr<Image2d>>& GetImageAttachments() const { return imageAttachments; }
    const std::vector<VkFramebuffer>&            GetFramebuffers() const { return framebuffers; }

private:
    const LogicalDevice& logicalDevice;

    std::vector<std::unique_ptr<Image2d>> imageAttachments;
    std::vector<VkFramebuffer>            framebuffers;
};
}   // namespace MapleLeaf