#include "Renderpass.hpp"
#include "Graphics.hpp"
#include "Log.hpp"
#include "LogicalDevice.hpp"
#include "RenderStage.hpp"


namespace MapleLeaf {
Renderpass::Renderpass(const LogicalDevice& logicalDevice, const RenderStage& renderStage, VkFormat depthFormat, VkFormat surfaceFormat,
                       VkSampleCountFlagBits samples)
    : logicalDevice(logicalDevice)
{
    std::vector<VkAttachmentDescription> attachmentDescriptions;

    for (const auto& attachment : renderStage.GetAttachments()) {
        auto attachmentSamples = attachment.IsMultisampled() ? samples : VK_SAMPLE_COUNT_1_BIT;

        VkAttachmentDescription attachmentDescription = {};
        attachmentDescription.samples                 = attachmentSamples;
        attachmentDescription.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;   // Clear at beginning of the render pass.
        // The image can be read from so it's important to store the attachment results
        attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;   // We don't care about initial layout of the attachment.

        switch (attachment.GetType()) {
        case Attachment::Type::Image:
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;   // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            attachmentDescription.format      = attachment.GetFormat();
            break;
        case Attachment::Type::Depth:
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;   // VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
            attachmentDescription.format      = depthFormat;
            break;
        case Attachment::Type::Swapchain:
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            attachmentDescription.format      = surfaceFormat;
            break;
        }

        attachmentDescriptions.emplace_back(attachmentDescription);
    }

    // Creates each subpass and its dependencies.
    std::vector<std::unique_ptr<SubpassDescription>> subpasses;
    std::vector<VkSubpassDependency>                 dependencies;

    for (const auto& subpassType : renderStage.GetSubpasses()) {
        if (subpassType.GetType() == SubpassType::Type::Compute) continue;
        
        std::vector<VkAttachmentReference> subpassOutputColorAttachments;
        std::vector<VkAttachmentReference> subpassInputColorAttachments;

        std::optional<uint32_t> depthAttachment;

        for (const auto& attachmentBinding : subpassType.GetInputAttachmentBindings()) {
            auto attachment = renderStage.GetAttachment(attachmentBinding);

            if (!attachment) {
                Log::Error("Failed to find a renderpass attachment bound to: ", attachmentBinding, '\n');
                continue;
            }

            VkAttachmentReference attachmentReference = {};
            attachmentReference.attachment            = attachment->GetBinding();
            attachmentReference.layout                = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;   // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            subpassInputColorAttachments.emplace_back(attachmentReference);
        }
        for (const auto& attachmentBinding : subpassType.GetOutputAttachmentBindings()) {
            auto attachment = renderStage.GetAttachment(attachmentBinding);

            if (!attachment) {
                Log::Error("Failed to find a renderpass attachment bound to: ", attachmentBinding, '\n');
                continue;
            }

            if (attachment->GetType() == Attachment::Type::Depth) {
                depthAttachment = attachment->GetBinding();
                continue;
            }

            VkAttachmentReference attachmentReference = {};
            attachmentReference.attachment            = attachment->GetBinding();
            attachmentReference.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;   // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            subpassOutputColorAttachments.emplace_back(attachmentReference);
        }

        // Subpass description.
        subpasses.emplace_back(std::make_unique<SubpassDescription>(
            VK_PIPELINE_BIND_POINT_GRAPHICS, subpassInputColorAttachments, subpassOutputColorAttachments, depthAttachment));

        // Subpass dependencies.
        VkSubpassDependency subpassDependency = {};

        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        subpassDependency.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependency.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
        subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        if (subpassType.GetBinding() == 0) {
            subpassDependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
            subpassDependency.srcStageMask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            subpassDependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            subpassDependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        }
        else {
            subpassDependency.srcSubpass = subpassType.GetBinding() - 1;
        }

        if (subpassType.GetBinding() == renderStage.GetSubpasses().size()) {
            subpassDependency.dstSubpass    = VK_SUBPASS_EXTERNAL;
            subpassDependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            subpassDependency.dstStageMask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            subpassDependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        }
        else {
            subpassDependency.dstSubpass = subpassType.GetBinding();
        }

        dependencies.emplace_back(subpassDependency);
    }

    std::vector<VkSubpassDescription> subpassDescriptions;
    subpassDescriptions.reserve(subpasses.size());

    for (const auto& subpass : subpasses) {
        subpassDescriptions.emplace_back(subpass->GetSubpassDescription());
    }

    // Creates the render pass.
    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount        = static_cast<uint32_t>(attachmentDescriptions.size());
    renderPassCreateInfo.pAttachments           = attachmentDescriptions.data();
    renderPassCreateInfo.subpassCount           = static_cast<uint32_t>(subpassDescriptions.size());
    renderPassCreateInfo.pSubpasses             = subpassDescriptions.data();
    renderPassCreateInfo.dependencyCount        = static_cast<uint32_t>(dependencies.size());
    renderPassCreateInfo.pDependencies          = dependencies.data();
    Graphics::CheckVk(vkCreateRenderPass(logicalDevice, &renderPassCreateInfo, nullptr, &renderpass));
}

Renderpass::~Renderpass()
{
    vkDestroyRenderPass(logicalDevice, renderpass, nullptr);
}
}   // namespace MapleLeaf