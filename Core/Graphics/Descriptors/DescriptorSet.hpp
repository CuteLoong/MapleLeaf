#pragma once

#include "CommandBuffer.hpp"
#include "Pipeline.hpp"

namespace MapleLeaf {
class DescriptorSet
{
public:
    explicit DescriptorSet(const Pipeline& pipeline);
    ~DescriptorSet();

    static void Update(const std::vector<VkWriteDescriptorSet>& descriptorWrites);
    void        BindDescriptor(const CommandBuffer& commandBuffer) const;

    const VkDescriptorSet& GetDescriptorSet(uint32_t setIndex) const { return descriptorSets.at(setIndex); }

private:
    VkPipelineLayout                    pipelineLayout;
    VkPipelineBindPoint                 pipelineBindPoint;
    VkDescriptorPool                    descriptorPool;
    std::map<uint32_t, VkDescriptorSet> descriptorSets;   // setIndex, DescriptorSet
};
}   // namespace MapleLeaf