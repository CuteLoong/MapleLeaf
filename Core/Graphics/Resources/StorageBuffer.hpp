#pragma once

#include "Buffer.hpp"
#include "Descriptor.hpp"

namespace MapleLeaf {
class StorageBuffer : public Descriptor, public Buffer
{
public:
    explicit StorageBuffer(VkDeviceSize size, const void* data = nullptr);

    void Update(const void* newData);

    WriteDescriptorSet GetWriteDescriptor(uint32_t binding, VkDescriptorType descriptorType,
                                          const std::optional<OffsetSize>& offsetSize) const override;

    static VkDescriptorSetLayoutBinding GetDescriptorSetLayout(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stage,
                                                               uint32_t count);
};
}   // namespace MapleLeaf