#include "ImageDepth.hpp"
#include <stdexcept>

namespace MapleLeaf {
static const std::vector<VkFormat> TRY_FORMATS = {
    VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};

ImageDepth::ImageDepth(const glm::uvec2& extent, VkSampleCountFlagBits samples)
    : Image(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, samples, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            FindSupportedFormat(TRY_FORMATS, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT), 1, 1, {extent.x, extent.y, 1})
{
    if (format == VK_FORMAT_UNDEFINED) throw std::runtime_error("No depth stencil format could be selected");
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (HasStencil(format)) aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

    CreateImage(
        image, memory, this->extent, format, samples, VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 1, VK_IMAGE_TYPE_2D);
    CreateImageSampler(sampler, filter, addressMode, false, 1);
    CreateImageView(image, view, VK_IMAGE_VIEW_TYPE_2D, format, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 0, 1, 0);
    // TransitionImageLayout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, layout, aspectMask, 1, 0, 1, 0);
}

void ImageDepth::CopyImageDepth(const CommandBuffer& commandBuffer, const ImageDepth& imageDepth) const
{
    InsertImageMemoryBarrier(commandBuffer,
                             image,
                             0,
                             VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                             1,
                             0,
                             1,
                             0);

    // Transition image from previous usage to transfer source layout
    InsertImageMemoryBarrier(commandBuffer,
                             imageDepth.image,
                             VK_ACCESS_MEMORY_READ_BIT,
                             VK_ACCESS_TRANSFER_READ_BIT,
                             imageDepth.layout,
                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                             1,
                             0,
                             1,
                             0);

    VkImageCopy imageCopyRegion{};
    imageCopyRegion.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    imageCopyRegion.srcSubresource.baseArrayLayer = 0;
    imageCopyRegion.srcSubresource.layerCount     = imageDepth.arrayLayers;
    imageCopyRegion.srcSubresource.mipLevel       = 0;
    imageCopyRegion.srcOffset                     = {0, 0, 0};
    imageCopyRegion.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    imageCopyRegion.dstSubresource.baseArrayLayer = 0;
    imageCopyRegion.dstSubresource.layerCount     = arrayLayers;
    imageCopyRegion.dstSubresource.mipLevel       = 0;
    imageCopyRegion.dstOffset                     = {0, 0, 0};
    imageCopyRegion.extent.width                  = std::min(extent.width, imageDepth.extent.width);
    imageCopyRegion.extent.height                 = std::min(extent.height, imageDepth.extent.height);
    imageCopyRegion.extent.depth                  = 1;

    vkCmdCopyImage(commandBuffer.GetCommandBuffer(),
                   imageDepth.image,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1,
                   &imageCopyRegion);

    // Transition destination image to general layout, which is the required layout for mapping the image memory later on.
    InsertImageMemoryBarrier(commandBuffer,
                             image,
                             VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_ACCESS_MEMORY_READ_BIT,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             layout,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                             1,
                             0,
                             1,
                             0);

    // Transition back the image after the blit is done.
    InsertImageMemoryBarrier(commandBuffer,
                             imageDepth.image,
                             VK_ACCESS_TRANSFER_READ_BIT,
                             VK_ACCESS_MEMORY_READ_BIT,
                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             imageDepth.layout,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                             1,
                             0,
                             1,
                             0);
}
}   // namespace MapleLeaf