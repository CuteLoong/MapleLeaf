#pragma once

#include "volk.h"
#include <memory>
#include <optional>

namespace MapleLeaf {
class OffsetSize
{
public:
    OffsetSize(uint32_t offset, uint32_t size)
        : offset(offset)
        , size(size)
    {}

    uint32_t GetOffset() const { return offset; }
    uint32_t GetSize() const { return size; }

    bool operator==(const OffsetSize& rhs) const { return offset == rhs.offset && size == rhs.size; }

    bool operator!=(const OffsetSize& rhs) const { return !operator==(rhs); }

private:
    uint32_t offset;
    uint32_t size;
};

class WriteDescriptorSet
{
public:
    WriteDescriptorSet(const VkWriteDescriptorSet& writeDescriptorSet, const VkDescriptorImageInfo& imageInfo)
        : writeDescriptorSet(writeDescriptorSet)
        , imageInfo(std::make_unique<VkDescriptorImageInfo>(imageInfo))
    {
        this->writeDescriptorSet.pImageInfo = this->imageInfo.get();
    }

    WriteDescriptorSet(const VkWriteDescriptorSet& writeDescriptorSet, const VkDescriptorBufferInfo& bufferInfo)
        : writeDescriptorSet(writeDescriptorSet)
        , bufferInfo(std::make_unique<VkDescriptorBufferInfo>(bufferInfo))
    {
        this->writeDescriptorSet.pBufferInfo = this->bufferInfo.get();
    }

    WriteDescriptorSet(const VkWriteDescriptorSet& writeDescriptorSet, const VkWriteDescriptorSetAccelerationStructureKHR& accelerationStructureInfo)
        : writeDescriptorSet(writeDescriptorSet)
        , accelerationStructureInfo(std::make_unique<VkWriteDescriptorSetAccelerationStructureKHR>(accelerationStructureInfo))
    {
        this->writeDescriptorSet.pNext = this->accelerationStructureInfo.get();
    }

    const VkWriteDescriptorSet& GetWriteDescriptorSet() const { return writeDescriptorSet; }

private:
    VkWriteDescriptorSet                                          writeDescriptorSet;
    std::unique_ptr<VkDescriptorImageInfo>                        imageInfo;
    std::unique_ptr<VkDescriptorBufferInfo>                       bufferInfo;
    std::unique_ptr<VkWriteDescriptorSetAccelerationStructureKHR> accelerationStructureInfo;
};

class Descriptor
{
public:
    Descriptor()          = default;
    virtual ~Descriptor() = default;

    virtual WriteDescriptorSet GetWriteDescriptor(uint32_t binding, VkDescriptorType descriptorType,
                                                  const std::optional<OffsetSize>& offsetSize) const = 0;
};
}   // namespace MapleLeaf