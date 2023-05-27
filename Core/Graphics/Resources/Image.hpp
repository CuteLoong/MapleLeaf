#pragma once

#include "CommandBuffer.hpp"
#include "Descriptor.hpp"
#include "glm/glm.hpp"
#include "volk.h"
#include <vector>

namespace MapleLeaf {

class Image : public Descriptor
{
public:
    Image(VkFilter filter, VkSamplerAddressMode addressMode, VkSampleCountFlagBits samples, VkImageLayout layout, VkImageUsageFlags usage,
          VkFormat format, uint32_t mipLevels, uint32_t arrayLayers, const VkExtent3D& extent);
    ~Image();

    WriteDescriptorSet                  GetWriteDescriptor(uint32_t binding, VkDescriptorType descriptorType,
                                                           const std::optional<OffsetSize>& offsetSize) const override;
    static VkDescriptorSetLayoutBinding GetDescriptorSetLayout(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stage,
                                                               uint32_t count);


    const VkExtent3D&     GetExtent() const { return extent; }
    glm::uvec2            GetSize() const { return {extent.width, extent.height}; }
    VkFormat              GetFormat() const { return format; }
    VkSampleCountFlagBits GetSamples() const { return samples; }
    VkImageUsageFlags     GetUsage() const { return usage; }
    uint32_t              GetMipLevels() const { return mipLevels; }
    uint32_t              GetArrayLevels() const { return arrayLayers; }
    VkFilter              GetFilter() const { return filter; }
    VkSamplerAddressMode  GetAddressMode() const { return addressMode; }
    VkImageLayout         GetLayout() const { return layout; }
    const VkImage&        GetImage() { return image; }
    const VkDeviceMemory& GetMemory() { return memory; }
    const VkSampler&      GetSampler() const { return sampler; }
    const VkImageView&    GetView() const { return view; }

    static uint32_t GetMipLevels(const VkExtent3D& extent);
    static uint32_t FindMemoryType(uint32_t typeFilter, const VkMemoryPropertyFlags& requiredProperties);
    static VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    static bool HasDepth(VkFormat format);
    static bool HasStencil(VkFormat format);

    static void CreateImage(VkImage& image, VkDeviceMemory& memory, const VkExtent3D& extent, VkFormat format, VkSampleCountFlagBits samples,
                            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t mipLevels, uint32_t arrayLayers,
                            VkImageType type);
    static void CreateImageSampler(VkSampler& sampler, VkFilter filter, VkSamplerAddressMode addressMode, bool anisotropic, uint32_t mipLevels);
    static void CreateImageView(const VkImage& image, VkImageView& imageView, VkImageViewType type, VkFormat format, VkImageAspectFlags imageAspect,
                                uint32_t mipLevels, uint32_t baseMipLevel, uint32_t layerCount, uint32_t baseArrayLayer);
    static void CreateMipmaps(const VkImage& image, const VkExtent3D& extent, VkFormat format, VkImageLayout dstImageLayout, uint32_t mipLevels,
                              uint32_t baseArrayLayer, uint32_t layerCount);
    static void TransitionImageLayout(const VkImage& image, VkFormat format, VkImageLayout srcImageLayout, VkImageLayout dstImageLayout,
                                      VkImageAspectFlags imageAspect, uint32_t mipLevels, uint32_t baseMipLevel, uint32_t layerCount,
                                      uint32_t baseArrayLayer);
    static void InsertImageMemoryBarrier(const CommandBuffer& commandBuffer, const VkImage& image, VkAccessFlags srcAccessMask,
                                         VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
                                         VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageAspectFlags imageAspect,
                                         uint32_t mipLevels, uint32_t baseMipLevel, uint32_t layerCount, uint32_t baseArrayLayer);
    static void CopyBufferToImage(const VkBuffer& buffer, const VkImage& image, const VkExtent3D& extent, uint32_t layerCount,
                                  uint32_t baseArrayLayer);
    static bool CopyImage(const VkImage& srcImage, VkImage& dstImage, VkDeviceMemory& dstImageMemory, VkFormat srcFormat, const VkExtent3D& extent,
                          VkImageLayout srcImageLayout, uint32_t mipLevel, uint32_t arrayLayer);

protected:
    VkExtent3D            extent;
    VkSampleCountFlagBits samples;
    VkImageUsageFlags     usage;
    VkFormat              format    = VK_FORMAT_UNDEFINED;
    uint32_t              mipLevels = 0;
    uint32_t              arrayLayers;

    VkFilter             filter;
    VkSamplerAddressMode addressMode;

    VkImageLayout layout;

    VkImage        image   = VK_NULL_HANDLE;
    VkDeviceMemory memory  = VK_NULL_HANDLE;
    VkSampler      sampler = VK_NULL_HANDLE;
    VkImageView    view    = VK_NULL_HANDLE;
};
}   // namespace MapleLeaf