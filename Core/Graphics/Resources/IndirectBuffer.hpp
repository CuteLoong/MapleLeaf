#pragma once

#include "Buffer.hpp"
#include "Descriptor.hpp"

namespace MapleLeaf {
class IndirectBuffer : public Descriptor, public Buffer
{
public:
    explicit IndirectBuffer(VkDeviceSize size, const void* data = nullptr);

    void Update(const void* newData);

    WriteDescriptorSet GetWriteDescriptor(uint32_t binding, VkDescriptorType descriptorType,
                                          const std::optional<OffsetSize>& offsetSize) const override;

    void IndirectBufferPipelineBarrier(const CommandBuffer& commandBuffer) const;

    static VkDescriptorSetLayoutBinding GetDescriptorSetLayout(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stage,
                                                               uint32_t count);
};
}   // namespace MapleLeaf