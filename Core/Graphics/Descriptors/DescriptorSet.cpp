#include "DescriptorSet.hpp"
#include "Graphics.hpp"

namespace MapleLeaf {
DescriptorSet::DescriptorSet(const Pipeline& pipeline)
    : pipelineLayout(pipeline.GetPipelineLayout())
    , pipelineBindPoint(pipeline.GetPipelineBindPoint())
    , descriptorPool(pipeline.GetDescriptorPool())
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    auto layouts = pipeline.GetDescriptorSetLayouts();

    for (const auto& [setIndex, layout] : layouts) {
        descriptorSets[setIndex] = VK_NULL_HANDLE;

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
        descriptorSetAllocateInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool              = descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount          = 1;
        descriptorSetAllocateInfo.pSetLayouts                 = &layout;
        Graphics::CheckVk(vkAllocateDescriptorSets(*logicalDevice, &descriptorSetAllocateInfo, &descriptorSets[setIndex]));
    }
}

DescriptorSet::~DescriptorSet()
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    for (auto& [setIndex, descriptorSet] : descriptorSets) Graphics::CheckVk(vkFreeDescriptorSets(*logicalDevice, descriptorPool, 1, &descriptorSet));
}

void DescriptorSet::Update(const std::vector<VkWriteDescriptorSet>& descriptorWrites)
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    vkUpdateDescriptorSets(*logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void DescriptorSet::BindDescriptor(const CommandBuffer& commandBuffer) const
{
    std::vector<VkDescriptorSet> descriptorSetsData;
    for (const auto& [setIndex, descriptorSet] : descriptorSets) descriptorSetsData.push_back(descriptorSet);

    if(!descriptorSetsData.empty()) vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, pipelineLayout, 0, descriptorSetsData.size(), descriptorSetsData.data(), 0, nullptr);
}
}   // namespace MapleLeaf