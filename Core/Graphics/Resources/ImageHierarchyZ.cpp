#include "ImageHierarchyZ.hpp"

#include "Graphics.hpp"

namespace MapleLeaf {
ImageHierarchyZ::ImageHierarchyZ(const glm::uvec2& extent, VkSampleCountFlagBits samples)
    : Image(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, samples, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_FORMAT_R32_SFLOAT, GetMipLevels(extent), 1,
            {extent.x, extent.y, 1})
{
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    CreateImage(image,
                memory,
                this->extent,
                format,
                samples,
                VK_IMAGE_TILING_OPTIMAL,
                usage,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                GetMipLevels(extent),
                1,
                VK_IMAGE_TYPE_2D);
    CreateImageSampler(sampler, filter, addressMode, false, GetMipLevels(extent));
    CreateImageView(image, view, VK_IMAGE_VIEW_TYPE_2D, format, VK_IMAGE_ASPECT_COLOR_BIT, GetMipLevels(extent), 0, 1, 0);
}

void ImageHierarchyZ::AddHierarchicalDepth(const CommandBuffer& commandBuffer, const VkImage& depth, const VkExtent3D& depthExtent,
                                           VkFormat depthFormat, VkImageLayout depthLayout, uint32_t hizMipLevel, uint32_t hizArrayLayer) const
{
    auto physicalDevice = Graphics::Get()->GetPhysicalDevice();

    auto supportsBlit = true;

    // Get device properites for the requested Image format.
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(*physicalDevice, format, &formatProperties);

    // Mip-chain generation requires support for blit source and destination
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) supportsBlit = false;
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) supportsBlit = false;

    InsertImageMemoryBarrier(commandBuffer,
                             depth,
                             VK_ACCESS_MEMORY_READ_BIT,
                             VK_ACCESS_TRANSFER_READ_BIT,
                             depthLayout,
                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_IMAGE_ASPECT_COLOR_BIT,
                             1,
                             0,
                             1,
                             0);

    InsertImageMemoryBarrier(commandBuffer,
                             this->image,
                             0,
                             VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_IMAGE_ASPECT_COLOR_BIT,
                             1,
                             hizMipLevel,
                             1,
                             hizArrayLayer);

    if (supportsBlit) {
        // Define the region to blit (we will blit the whole swapchain image).
        VkOffset3D blitSize = {static_cast<int32_t>(extent.width), static_cast<int32_t>(extent.height), static_cast<int32_t>(extent.depth)};

        VkImageBlit imageBlitRegion                   = {};
        imageBlitRegion.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.srcSubresource.mipLevel       = 0;
        imageBlitRegion.srcSubresource.baseArrayLayer = 0;
        imageBlitRegion.srcSubresource.layerCount     = 1;
        imageBlitRegion.srcOffsets[1]                 = blitSize;
        imageBlitRegion.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.dstSubresource.mipLevel       = hizMipLevel;
        imageBlitRegion.dstSubresource.baseArrayLayer = hizArrayLayer;
        imageBlitRegion.dstSubresource.layerCount     = 1;
        imageBlitRegion.dstOffsets[1]                 = blitSize;
        vkCmdBlitImage(commandBuffer,
                       depth,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       this->image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1,
                       &imageBlitRegion,
                       VK_FILTER_NEAREST);
    }
    else {
        // Otherwise use image copy (requires us to manually flip components).
        VkImageCopy imageCopyRegion                   = {};
        imageCopyRegion.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.mipLevel       = 0;
        imageCopyRegion.srcSubresource.baseArrayLayer = 0;
        imageCopyRegion.srcSubresource.layerCount     = 1;
        imageCopyRegion.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.mipLevel       = hizMipLevel;
        imageCopyRegion.dstSubresource.baseArrayLayer = hizArrayLayer;
        imageCopyRegion.dstSubresource.layerCount     = 1;
        imageCopyRegion.extent                        = extent;
        vkCmdCopyImage(
            commandBuffer, depth, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, this->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
    }

    // Transition destination image to general layout, which is the required layout for mapping the image memory later on.
    InsertImageMemoryBarrier(commandBuffer,
                             this->image,
                             VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_ACCESS_MEMORY_READ_BIT,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             this->layout,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_IMAGE_ASPECT_COLOR_BIT,
                             1,
                             hizMipLevel,
                             1,
                             hizArrayLayer);

    // Transition back the image after the blit is done.
    InsertImageMemoryBarrier(commandBuffer,
                             depth,
                             VK_ACCESS_TRANSFER_READ_BIT,
                             VK_ACCESS_MEMORY_READ_BIT,
                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             depthLayout,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_IMAGE_ASPECT_COLOR_BIT,
                             1,
                             0,
                             1,
                             0);
}


uint32_t ImageHierarchyZ::GetMipLevels(const glm::uvec2& extent)
{
    return std::floor(log(std::max(extent.x, extent.y)));
}
}   // namespace MapleLeaf