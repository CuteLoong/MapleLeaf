#include "HiZDrawSubrender.hpp"

#include "Graphics.hpp"

namespace MapleLeaf {
HiZDrawSubrender::HiZDrawSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline("Shader/GPUDriven/HierarchicalDepth.comp")
{
    hiz = std::make_unique<ImageHierarchyZ>(glm::uvec2(Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y));

    hierarchyDepth = std::make_unique<Image2d>(glm::uvec2(Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y),
                                               VK_FORMAT_R32_SFLOAT,
                                               VK_IMAGE_LAYOUT_GENERAL,
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                               VK_FILTER_LINEAR,
                                               VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                               VK_SAMPLE_COUNT_1_BIT);
}

void HiZDrawSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void HiZDrawSubrender::Render(const CommandBuffer& commandBuffer) {}

void HiZDrawSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    glm::uvec2 previousLevelDimensions, currentDimensions;
    int        mipLevel     = 0;
    previousLevelDimensions = {Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y};
    currentDimensions       = {Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y};


    pipeline.BindPipeline(commandBuffer);

    pushHandler.Push("previousLevelDimensions", previousLevelDimensions);
    pushHandler.Push("currentDimensions", currentDimensions);
    pushHandler.Push("mipLevel", mipLevel);

    descriptorSet.Push("HiZ", hierarchyDepth);
    descriptorSet.Push("depthBuffer", Graphics::Get()->GetAttachment("depth"));
    descriptorSet.Push("PushObject", pushHandler);
    if (!descriptorSet.Update(pipeline)) return;

    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    pushHandler.BindPush(commandBuffer, pipeline);

    pipeline.CmdRender(commandBuffer, glm::uvec2(currentDimensions.x, currentDimensions.y));

    hiz->AddHierarchicalDepth(commandBuffer,
                              hierarchyDepth->GetImage(),
                              {currentDimensions.x, currentDimensions.y, 0},
                              VK_FORMAT_R32_SFLOAT,
                              VK_IMAGE_LAYOUT_GENERAL,
                              0,
                              0);
}
}   // namespace MapleLeaf