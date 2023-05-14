#include "Image.hpp"
#include "Graphics.hpp"

namespace MapleLeaf {
constexpr static float ANISOTROPY = 16.0f;
Image::Image(VkFilter filter, VkSamplerAddressMode addressMode, VkSampleCountFlagBits samples, VkImageLayout layout, VkImageUsageFlags usage,
             VkFormat format, uint32_t mipLevels, uint32_t arrayLayers, const VkExtent3D& extent)
    : extent(extent)
    , samples(samples)
    , usage(usage)
    , format(format)
    , mipLevels(mipLevels)
    , arrayLayers(arrayLayers)
    , filter(filter)
    , addressMode(addressMode)
    , layout(layout)
{}

Image::~Image()
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    vkDestroyImageView(*logicalDevice, view, nullptr);
    vkDestroySampler(*logicalDevice, sampler, nullptr);
    vkFreeMemory(*logicalDevice, memory, nullptr);
    vkDestroyImage(*logicalDevice, image, nullptr);
}

uint32_t Image::GetMipLevels(const VkExtent3D& extent)
{
    return static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, std::max(extent.height, extent.depth)))) + 1);
}

uint32_t Image::FindMemoryType(uint32_t typeFilter, const VkMemoryPropertyFlags& requiredProperties)
{
    auto physicalDevice   = Graphics::Get()->GetPhysicalDevice();
    auto memoryProperties = physicalDevice->GetMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        uint32_t memoryTypeBits = 1 << i;

        if (typeFilter & memoryTypeBits && (memoryProperties.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find a valid memory type for buffer");
}

VkFormat Image::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    auto physicalDevice = Graphics::Get()->GetPhysicalDevice();

    for (const auto& format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(*physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) return format;
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) return format;
    }

    return VK_FORMAT_UNDEFINED;
}

bool Image::HasDepth(VkFormat format)
{
    static const std::vector<VkFormat> DEPTH_FORMATS = {VK_FORMAT_D16_UNORM,
                                                        VK_FORMAT_X8_D24_UNORM_PACK32,
                                                        VK_FORMAT_D32_SFLOAT,
                                                        VK_FORMAT_D16_UNORM_S8_UINT,
                                                        VK_FORMAT_D24_UNORM_S8_UINT,
                                                        VK_FORMAT_D32_SFLOAT_S8_UINT};
    return std::find(DEPTH_FORMATS.begin(), DEPTH_FORMATS.end(), format) != std::end(DEPTH_FORMATS);
}

bool Image::HasStencil(VkFormat format)
{
    static const std::vector<VkFormat> STENCIL_FORMATS = {
        VK_FORMAT_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT};
    return std::find(STENCIL_FORMATS.begin(), STENCIL_FORMATS.end(), format) != std::end(STENCIL_FORMATS);
}

void Image::CreateImage(VkImage& image, VkDeviceMemory& memory, const VkExtent3D& extent, VkFormat format, VkSampleCountFlagBits samples,
                        VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t mipLevels, uint32_t arrayLayers,
                        VkImageType type)
{
    auto              logicalDevice   = Graphics::Get()->GetLogicalDevice();
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.flags             = arrayLayers == 6 ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
    imageCreateInfo.imageType         = type;
    imageCreateInfo.format            = format;
    imageCreateInfo.extent            = extent;
    imageCreateInfo.mipLevels         = mipLevels;
    imageCreateInfo.arrayLayers       = arrayLayers;
    imageCreateInfo.samples           = samples;
    imageCreateInfo.tiling            = tiling;
    imageCreateInfo.usage             = usage;
    imageCreateInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    Graphics::CheckVk(vkCreateImage(*logicalDevice, &imageCreateInfo, nullptr, &image));

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(*logicalDevice, image, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize       = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex      = FindMemoryType(memoryRequirements.memoryTypeBits, properties);
    Graphics::CheckVk(vkAllocateMemory(*logicalDevice, &memoryAllocateInfo, nullptr, &memory));

    Graphics::CheckVk(vkBindImageMemory(*logicalDevice, image, memory, 0));
}

void Image::CreateImageSampler(VkSampler& sampler, VkFilter filter, VkSamplerAddressMode addressMode, bool anisotropic, uint32_t mipLevels)
{
    auto physicalDevice = Graphics::Get()->GetPhysicalDevice();
    auto logicalDevice  = Graphics::Get()->GetLogicalDevice();

    VkSamplerCreateInfo samplerCreateInfo     = {};
    samplerCreateInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter               = filter;
    samplerCreateInfo.minFilter               = filter;
    samplerCreateInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU            = addressMode;
    samplerCreateInfo.addressModeV            = addressMode;
    samplerCreateInfo.addressModeW            = addressMode;
    samplerCreateInfo.mipLodBias              = 0.0f;
    samplerCreateInfo.anisotropyEnable        = static_cast<VkBool32>(anisotropic);
    samplerCreateInfo.maxAnisotropy           = (anisotropic && logicalDevice->GetEnabledFeatures().samplerAnisotropy)
                                                    ? std::min(ANISOTROPY, physicalDevice->GetProperties().limits.maxSamplerAnisotropy)
                                                    : 1.0f;
    samplerCreateInfo.minLod                  = 0.0f;
    samplerCreateInfo.maxLod                  = static_cast<float>(mipLevels);
    samplerCreateInfo.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    Graphics::CheckVk(vkCreateSampler(*logicalDevice, &samplerCreateInfo, nullptr, &sampler));
}

void Image::CreateImageView(const VkImage& image, VkImageView& imageView, VkImageViewType type, VkFormat format, VkImageAspectFlags imageAspect,
                            uint32_t mipLevels, uint32_t baseMipLevel, uint32_t layerCount, uint32_t baseArrayLayer)
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image                 = image;
    imageViewCreateInfo.viewType              = type;
    imageViewCreateInfo.format                = format;
    imageViewCreateInfo.components            = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
    imageViewCreateInfo.subresourceRange.aspectMask     = imageAspect;
    imageViewCreateInfo.subresourceRange.baseMipLevel   = baseMipLevel;
    imageViewCreateInfo.subresourceRange.levelCount     = mipLevels;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
    imageViewCreateInfo.subresourceRange.layerCount     = layerCount;
    Graphics::CheckVk(vkCreateImageView(*logicalDevice, &imageViewCreateInfo, nullptr, &imageView));
}

// void Image::CreateMipmaps(const VkImage& image, const VkExtent3D& extent, VkFormat format, VkImageLayout dstImageLayout, uint32_t mipLevels,
//                           uint32_t baseArrayLayer, uint32_t layerCount)
// {
//     auto physicalDevice = Graphics::Get()->GetPhysicalDevice();

//     // Get device properites for the requested Image format.
//     VkFormatProperties formatProperties;
//     vkGetPhysicalDeviceFormatProperties(*physicalDevice, format, &formatProperties);

//     // Mip-chain generation requires support for blit source and destination
//     assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT);
//     assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT);

// 	CommandBuffer commandBuffer;


// }
}   // namespace MapleLeaf