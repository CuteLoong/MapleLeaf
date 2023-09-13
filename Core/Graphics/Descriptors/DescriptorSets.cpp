#include "DescriptorSets.hpp"
#include "Graphics.hpp"

namespace MapleLeaf {
DescriptorSets::DescriptorSets(const Pipeline& pipeline)
    : pipelineLayout(pipeline.GetPipelineLayout())
    , pipelineBindPoint(pipeline.GetPipelineBindPoint())
    , descriptorPool(pipeline.GetDescriptorPool())
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    auto     normalLayouts   = pipeline.GetNormalDescriptorSetLayouts();
    auto     bindlessLayouts = pipeline.GetBindlessDescriptorSetLayouts();
    uint32_t NumDescriptors  = logicalDevice->GetBindlessMaxDescriptorsCount();

    for (const auto& [setIndex, layout] : normalLayouts) {
        descriptorSets[setIndex] = VK_NULL_HANDLE;

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
        descriptorSetAllocateInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool              = descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount          = 1;
        descriptorSetAllocateInfo.pSetLayouts                 = &layout;
        Graphics::CheckVk(vkAllocateDescriptorSets(*logicalDevice, &descriptorSetAllocateInfo, &descriptorSets[setIndex]));
    }

    for (const auto& [setIndex, layout] : bindlessLayouts) {
        descriptorSets[setIndex] = VK_NULL_HANDLE;

        VkDescriptorSetVariableDescriptorCountAllocateInfoEXT variableInfo{};
        variableInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
        variableInfo.descriptorSetCount = 1;
        variableInfo.pDescriptorCounts  = &NumDescriptors;

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
        descriptorSetAllocateInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool              = descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount          = 1;
        descriptorSetAllocateInfo.pSetLayouts                 = &layout;
        descriptorSetAllocateInfo.pNext                       = &variableInfo;
        Graphics::CheckVk(vkAllocateDescriptorSets(*logicalDevice, &descriptorSetAllocateInfo, &descriptorSets[setIndex]));
    }
}

DescriptorSets::~DescriptorSets()
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    for (auto& [setIndex, descriptorSet] : descriptorSets) Graphics::CheckVk(vkFreeDescriptorSets(*logicalDevice, descriptorPool, 1, &descriptorSet));
}

void DescriptorSets::Update(const std::vector<VkWriteDescriptorSet>& descriptorWrites)
{
    auto logicalDevice = Graphics::Get()->GetLogicalDevice();

    vkUpdateDescriptorSets(*logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void DescriptorSets::BindDescriptor(const CommandBuffer& commandBuffer) const
{
    std::vector<VkDescriptorSet> descriptorSetsData;
    for (const auto& [setIndex, descriptorSet] : descriptorSets) descriptorSetsData.push_back(descriptorSet);

    if (!descriptorSetsData.empty())
        vkCmdBindDescriptorSets(
            commandBuffer, pipelineBindPoint, pipelineLayout, 0, descriptorSetsData.size(), descriptorSetsData.data(), 0, nullptr);
}
}   // namespace MapleLeaf