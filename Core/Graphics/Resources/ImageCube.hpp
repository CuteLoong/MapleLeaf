#pragma once

#include "Bitmap.hpp"
#include "Image.hpp"
#include <filesystem>

namespace MapleLeaf {
class ImageCube : public Image
{
public:
    static std::shared_ptr<ImageCube> Create(const std::filesystem::path& filename, VkFilter filter = VK_FILTER_LINEAR,
                                             VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, bool anisotropic = true,
                                             bool mipmap = true);

    explicit ImageCube(const glm::uvec2& extent, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
                       VkImageLayout     layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VkFilter filter = VK_FILTER_LINEAR,
                       VkSamplerAddressMode  addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                       VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, bool anisotropic = false, bool mipmap = false);

    explicit ImageCube(std::filesystem::path filename, VkFilter filter = VK_FILTER_LINEAR,
                       VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, bool anisotropic = true, bool mipmap = true,
                       bool load = true);

private:
    void Load(std::unique_ptr<Bitmap> loadBitmap = nullptr);

    std::filesystem::path filename;

    bool     anisotropic;
    bool     mipmap;
    uint32_t components = 0;
};
}   // namespace MapleLeaf