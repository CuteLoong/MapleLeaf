#include "Image2d.hpp"

namespace MapleLeaf {
Image2d::Image2d(const glm::uvec2& extent, VkFormat format, VkImageLayout layout, VkImageUsageFlags usage, VkFilter filter,
                 VkSamplerAddressMode addressMode, VkSampleCountFlagBits samples, bool anisotropic, bool mipmap)
    : Image(filter, addressMode, samples, layout,
            usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, format, 1, 1,
            {extent.x, extent.y, 1})
    , anisotropic(anisotropic)
    , mipmap(mipmap)
    , components(4)
{
    Image2d::Load();
}

void Image2d::Load()
{
    if (extent.width == 0 || extent.height == 0) return;

    mipLevels = mipmap ? GetMipLevels(extent) : 1;

    CreateImage(image,
                memory,
                extent,
                format,
                samples,
                VK_IMAGE_TILING_OPTIMAL,
                usage,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                mipLevels,
                arrayLayers,
                VK_IMAGE_TYPE_2D);
    CreateImageSampler(sampler, filter, addressMode, anisotropic, mipLevels);
    CreateImageView(image, view, VK_IMAGE_VIEW_TYPE_2D, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, 0, arrayLayers, 0);

    // if (mipmap) {
    //     CreateMipmaps(image, extent, format, layout, mipLevels, 0, arrayLayers);
    // }
}
}   // namespace MapleLeaf