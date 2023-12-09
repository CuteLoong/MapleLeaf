#include "NonRTAttachmentsHandler.hpp"

#include "Devices.hpp"
#include "Graphics.hpp"

namespace MapleLeaf {
NonRTAttachmentsHandler::NonRTAttachmentsHandler(const std::vector<NonRTAttachment>& nonRTAttachments)
    : nonRTAttachments(nonRTAttachments)
{}

void NonRTAttachmentsHandler::Update()
{
    if (frameAttachmentSize != Devices::Get()->GetWindow()->GetSize()) {
        frameAttachmentSize = Devices::Get()->GetWindow()->GetSize();
        RecreateAttachments();
    }
}

void NonRTAttachmentsHandler::RecreateAttachments()
{
    auto physicalDevice = Graphics::Get()->GetPhysicalDevice();
    auto logicalDevice  = Graphics::Get()->GetLogicalDevice();
    auto graphicsQueue  = logicalDevice->GetGraphicsQueue();

    Graphics::CheckVk(vkQueueWaitIdle(graphicsQueue));
    NonRTImages.clear();

    for (auto& nonRTAttachment : nonRTAttachments) {
        if (nonRTAttachment.fixedSize != std::nullopt) continue;

        auto attachmentSamples = nonRTAttachment.IsMultisampled() ? physicalDevice->GetMsaaSamples() : VK_SAMPLE_COUNT_1_BIT;

        switch (nonRTAttachment.GetType()) {
        case NonRTAttachment::Type::Image2d:
            NonRTImages.emplace(
                nonRTAttachment.GetName(),
                std::make_unique<Image2d>(frameAttachmentSize,
                                          nonRTAttachment.GetFormat(),
                                          VK_IMAGE_LAYOUT_GENERAL,   // here is VK_IMAGE_LAYOUT_GENERAL, because we will use it as write output image
                                          VK_IMAGE_USAGE_STORAGE_BIT,
                                          nonRTAttachment.GetFilter(),
                                          VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                          attachmentSamples));
            break;
        case NonRTAttachment::Type::ImageCube:
            NonRTImages.emplace(nonRTAttachment.GetName(),
                                std::make_unique<ImageCube>(frameAttachmentSize,
                                                            nonRTAttachment.GetFormat(),
                                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                                                            nonRTAttachment.GetFilter(),
                                                            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                            attachmentSamples));
            break;
        case NonRTAttachment::Type::ImageHierarchyZ:
            NonRTImages.emplace(nonRTAttachment.GetName(), std::make_unique<ImageHierarchyZ>(frameAttachmentSize));
            break;
        case NonRTAttachment::Type::StereoImageHierarchyZ:
            NonRTImages.emplace(nonRTAttachment.GetName(), std::make_unique<ImageHierarchyZ>(Devices::Get()->GetWindow()->GetStereoSize()));
            break;
        }
    }
}
}   // namespace MapleLeaf