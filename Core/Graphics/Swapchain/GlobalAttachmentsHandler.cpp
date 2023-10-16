#include "GlobalAttachmentsHandler.hpp"

#include "Devices.hpp"
#include "Graphics.hpp"

namespace MapleLeaf {
GlobalAttachmentsHandler::GlobalAttachmentsHandler(const std::vector<FrameAttachment>& frameAttachmentTypes)
    : frameAttachmentTypes(frameAttachmentTypes)
{}

void GlobalAttachmentsHandler::Update()
{
    if (frameAttachmentSize != Devices::Get()->GetWindow()->GetSize()) {
        frameAttachmentSize = Devices::Get()->GetWindow()->GetSize();
        RecreateAttachments();
    }
}

void GlobalAttachmentsHandler::RecreateAttachments()
{
    auto physicalDevice = Graphics::Get()->GetPhysicalDevice();
    frameAttachments.clear();

    for (auto& frameAttachmentType : frameAttachmentTypes) {
        if (frameAttachmentType.fixedSize != std::nullopt) continue;

        auto attachmentSamples = frameAttachmentType.IsMultisampled() ? physicalDevice->GetMsaaSamples() : VK_SAMPLE_COUNT_1_BIT;

        switch (frameAttachmentType.GetType()) {
        case FrameAttachment::Type::Image2d:
            frameAttachments.emplace(
                frameAttachmentType.GetName(),
                std::make_unique<Image2d>(frameAttachmentSize,
                                          frameAttachmentType.GetFormat(),
                                          VK_IMAGE_LAYOUT_GENERAL,   // here is VK_IMAGE_LAYOUT_GENERAL, because we will use it as write output image
                                          VK_IMAGE_USAGE_STORAGE_BIT,
                                          frameAttachmentType.GetFilter(),
                                          VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                          attachmentSamples));
            break;
        case FrameAttachment::Type::ImageCube:
            frameAttachments.emplace(frameAttachmentType.GetName(),
                                     std::make_unique<ImageCube>(frameAttachmentSize,
                                                                 frameAttachmentType.GetFormat(),
                                                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                                                                 frameAttachmentType.GetFilter(),
                                                                 VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                                 attachmentSamples));
            break;
        case FrameAttachment::Type::ImageHierarchyZ:
            frameAttachments.emplace(frameAttachmentType.GetName(), std::make_unique<ImageHierarchyZ>(frameAttachmentSize));
            break;
        }
    }
}
}   // namespace MapleLeaf