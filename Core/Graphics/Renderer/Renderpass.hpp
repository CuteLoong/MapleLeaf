#pragma once

#include "volk.h"
#include <optional>
#include <vector>

#include "NonCopyable.hpp"
#include "vulkan/vulkan_core.h"

namespace MapleLeaf {
class LogicalDevice;
class ImageDepth;
class RenderStage;

class Renderpass
{
public:
    class SubpassDescription : NonCopyable
    {
    public:
        SubpassDescription(VkPipelineBindPoint bindPoint, std::vector<VkAttachmentReference> colorOutputAttachments,
                           std::vector<VkAttachmentReference> colorInputAttachments, const std::optional<uint32_t>& depthAttachment)
            : colorOutputAttachments(std::move(colorOutputAttachments))
            , colorInputAttachments(std::move(colorInputAttachments))
        {
            subpassDescription.pipelineBindPoint    = bindPoint;
            subpassDescription.colorAttachmentCount = static_cast<uint32_t>(this->colorOutputAttachments.size());
            subpassDescription.pColorAttachments    = this->colorOutputAttachments.data();
            subpassDescription.inputAttachmentCount = static_cast<uint32_t>(this->colorInputAttachments.size());
            subpassDescription.pInputAttachments    = this->colorInputAttachments.data();

            if (depthAttachment) {
                depthStencilAttachment.attachment          = *depthAttachment;
                depthStencilAttachment.layout              = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                subpassDescription.pDepthStencilAttachment = &depthStencilAttachment;
            }
        }

        const VkSubpassDescription& GetSubpassDescription() const { return subpassDescription; }

    private:
        VkSubpassDescription               subpassDescription = {};
        std::vector<VkAttachmentReference> colorOutputAttachments;
        std::vector<VkAttachmentReference> colorInputAttachments;
        VkAttachmentReference              depthStencilAttachment = {};
    };

    Renderpass(const LogicalDevice& logicalDevice, const RenderStage& renderStage, VkFormat depthFormat, VkFormat surfaceFormat,
               VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    ~Renderpass();

    operator const VkRenderPass&() const { return renderpass; }
    const VkRenderPass& GetRenderpass() const { return renderpass; }

private:
    const LogicalDevice& logicalDevice;

    VkRenderPass renderpass = VK_NULL_HANDLE;
};
}   // namespace MapleLeaf