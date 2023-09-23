#include "Image2d.hpp"
#include "Buffer.hpp"

namespace MapleLeaf {
std::shared_ptr<Image2d> Image2d::Create(const std::filesystem::path& filename, VkFilter filter, VkSamplerAddressMode addressMode, bool anisotropic,
                                         bool mipmap)
{
    auto result = std::make_shared<Image2d>(filename, filter, addressMode, anisotropic, mipmap, false);
    result->Load();
    return result;
}

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

Image2d::Image2d(std::filesystem::path filename, VkFilter filter, VkSamplerAddressMode addressMode, bool anisotropic, bool mipmap, bool load)
    : Image(filter, addressMode, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R8G8B8A8_UNORM, 1, 1, {0, 0, 1})
    , filename(std::move(filename))
    , anisotropic(anisotropic)
    , mipmap(mipmap)
{
    if (load) {
        Image2d::Load();
    }
}

Image2d::Image2d(std::unique_ptr<Bitmap>&& bitmap, VkFormat format, VkImageLayout layout, VkImageUsageFlags usage, VkFilter filter,
                 VkSamplerAddressMode addressMode, VkSampleCountFlagBits samples, bool anisotropic, bool mipmap)
    : Image(filter, addressMode, samples, layout,
            usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, format, 1, 1,
            {bitmap->GetSize().x, bitmap->GetSize().y, 1})
    , anisotropic(anisotropic)
    , mipmap(mipmap)
    , components(bitmap->GetBytesPerPixel())
{
    Image2d::Load(std::move(bitmap));
}


void Image2d::Load(std::unique_ptr<Bitmap> loadBitmap)
{
    if (!filename.empty() && !loadBitmap) {
        loadBitmap = std::make_unique<Bitmap>(filename);
        extent     = {loadBitmap->GetSize().y, loadBitmap->GetSize().y, 1};
        components = loadBitmap->GetBytesPerPixel();
    }

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

    if (loadBitmap || mipmap) {
        TransitionImageLayout(
            image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, 0, arrayLayers, 0);
    }

    if (loadBitmap) {
        Buffer bufferStaging(
            loadBitmap->GetLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        uint8_t* data;
        bufferStaging.MapMemory(reinterpret_cast<void**>(&data));
        std::memcpy(data, loadBitmap->GetData().get(), bufferStaging.GetSize());
        bufferStaging.UnmapMemory();

        CopyBufferToImage(bufferStaging.GetBuffer(), image, extent, arrayLayers, 0);
    }

    if (mipmap) {
        CreateMipmaps(image, extent, format, layout, mipLevels, 0, arrayLayers);
    }
    else if (loadBitmap) {
        TransitionImageLayout(image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layout, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, 0, arrayLayers, 0);
    }
    else {
        TransitionImageLayout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, layout, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, 0, arrayLayers, 0);
    }
}
}   // namespace MapleLeaf