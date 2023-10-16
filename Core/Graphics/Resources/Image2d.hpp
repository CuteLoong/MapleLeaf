#pragma once

#include "Bitmap.hpp"
#include "Image.hpp"
#include <filesystem>


namespace MapleLeaf {
class Image2d : public Image
{
public:
    static std::shared_ptr<Image2d> Create(const std::filesystem::path& filename, VkFilter filter = VK_FILTER_LINEAR,
                                           VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT, bool anisotropic = true,
                                           bool mipmap = false);

    /**
     * Creates a new 2D image.
     * @param extent The images extent in pixels.
     * @param format The format and type of the texel blocks that will be contained in the image.
     * @param layout The layout that the image subresources accessible from.
     * @param usage The intended usage of the image.
     * @param filter The magnification/minification filter to apply to lookups.
     * @param addressMode The addressing mode for outside [0..1] range.
     * @param samples The number of samples per texel.
     * @param anisotropic If anisotropic filtering is enabled.
     * @param mipmap If mapmaps will be generated.
     */
    explicit Image2d(const glm::uvec2& extent, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
                     VkImageLayout     layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VkFilter filter = VK_FILTER_LINEAR,
                     VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
                     bool anisotropic = false, bool mipmap = false);

    /**
     * Creates a new 2D image.
     * @param filename The file to load the image from.
     * @param filter The magnification/minification filter to apply to lookups.
     * @param addressMode The addressing mode for outside [0..1] range.
     * @param anisotropic If anisotropic filtering is enabled.
     * @param mipmap If mapmaps will be generated.
     * @param load If this resource will be loaded immediately, otherwise {@link Image2d#Load} can be called later.
     */
    explicit Image2d(std::filesystem::path filename, VkFilter filter = VK_FILTER_LINEAR,
                     VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT, bool anisotropic = true, bool mipmap = true,
                     bool load = true);

    /**
     * Creates a new 2D image.
     * @param bitmap The bitmap to load from.
     * @param format The format and type of the texel blocks that will be contained in the image.
     * @param layout The layout that the image subresources accessible from.
     * @param usage The intended usage of the image.
     * @param filter The magnification/minification filter to apply to lookups.
     * @param addressMode The addressing mode for outside [0..1] range.
     * @param samples The number of samples per texel.
     * @param anisotropic If anisotropic filtering is enabled.
     * @param mipmap If mapmaps will be generated.
     */
    explicit Image2d(std::unique_ptr<Bitmap>&& bitmap, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
                     VkImageLayout     layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VkFilter filter = VK_FILTER_LINEAR,
                     VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
                     bool anisotropic = false, bool mipmap = false);

    void Image2dPipelineBarrierComputeToCompute(const CommandBuffer& commandBuffer) const;
    void Image2dPipelineBarrierComputeToGraphic(const CommandBuffer& commandBuffer) const;
    void Image2dPipelineBarrierGraphicToCompute(const CommandBuffer& commandBuffer) const;

private:
    void Load(std::unique_ptr<Bitmap> loadBitmap = nullptr);

    std::filesystem::path filename;

    bool     anisotropic;
    bool     mipmap;
    uint32_t components = 0;
};
}   // namespace MapleLeaf