#include "IndirectBuffer.hpp"

namespace MapleLeaf {
IndirectBuffer::IndirectBuffer(VkDeviceSize size, const void* data)
    : Buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data)
{}

void IndirectBuffer::Update(const void* newData)
{
    void* data;
    MapMemory(&data);
    std::memcpy(data, newData, static_cast<std::size_t>(size));
    UnmapMemory();
}

WriteDescriptorSet IndirectBuffer::GetWriteDescriptor(uint32_t binding, VkDescriptorType descriptorType,
                                                      const std::optional<OffsetSize>& offsetSize) const
{
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer                 = buffer;
    bufferInfo.offset                 = 0;
    bufferInfo.range                  = size;

    if (offsetSize) {
        bufferInfo.offset = offsetSize->GetOffset();
        bufferInfo.range  = offsetSize->GetSize();
    }

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet               = VK_NULL_HANDLE;   // Will be set in the descriptor handler.
    descriptorWrite.dstBinding           = binding;
    descriptorWrite.dstArrayElement      = 0;                // Will be set in the descriptor handler.
    descriptorWrite.descriptorCount      = 1;
    descriptorWrite.descriptorType       = descriptorType;

    return {descriptorWrite, bufferInfo};
}

VkDescriptorSetLayoutBinding IndirectBuffer::GetDescriptorSetLayout(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stage,
                                                                    uint32_t count)
{
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
    descriptorSetLayoutBinding.binding                      = binding;
    descriptorSetLayoutBinding.descriptorType               = descriptorType;
    descriptorSetLayoutBinding.descriptorCount              = count;
    descriptorSetLayoutBinding.stageFlags                   = stage;
    descriptorSetLayoutBinding.pImmutableSamplers           = nullptr;
    return descriptorSetLayoutBinding;
}
}   // namespace MapleLeaf