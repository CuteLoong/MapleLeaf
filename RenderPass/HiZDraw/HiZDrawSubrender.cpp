#include "HiZDrawSubrender.hpp"

#include "Graphics.hpp"
#include "Image2d.hpp"
#include "ImageHierarchyZ.hpp"
#include "PipelineGraphics.hpp"
#include "Scenes.hpp"
#include "glm/fwd.hpp"
#include "vulkan/vulkan_core.h"
#include <memory>
#include <stdint.h>

namespace MapleLeaf {
HiZDrawSubrender::HiZDrawSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineCompute1("Shader/GPUDriven/culling.comp")
    , pipelineCompute2("Shader/GPUDriven/HierarchicalDepth-Max.comp")
    , PipelineGraphics(pipelineStage, {"Shader/GPUDriven/TwoPass.vert", "Shader/GPUDriven/TwoPass.frag"}, {Vertex3D::GetVertexInput()}, {},
                       PipelineGraphics::Mode::MRT)
{
    RecreateHiDepths();
}

void HiZDrawSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    RecreateHiDepths();
    // fisrt-pass compute pipeline
    const auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    if (!gpuScene) return;
    if (!gpuScene->GetIndirectBuffer()) return;

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCameraCompute);

    uint32_t instanceCount = gpuScene->GetInstanceCount();

    pushHandlerFirstDraw.Push("instanceCount", instanceCount);

    descriptorSetCompute1.Push("InstanceDatas", gpuScene->GetInstanceDatasHandler());
    descriptorSetCompute1.Push("DrawCommandBuffer", gpuScene->GetIndirectBuffer());
    descriptorSetCompute1.Push("UniformCamera", uniformCameraCompute);
    descriptorSetCompute1.Push("PushObject", pushHandlerFirstDraw);

    if (!descriptorSetCompute1.Update(pipelineCompute1)) return;
    pipelineCompute1.BindPipeline(commandBuffer);
    descriptorSetCompute1.BindDescriptor(commandBuffer, pipelineCompute1);
    pushHandlerFirstDraw.BindPush(commandBuffer, pipelineCompute1);
    pipelineCompute1.CmdRender(commandBuffer, glm::uvec2(instanceCount, 1));

    gpuScene->GetIndirectBuffer()->IndirectBufferPipelineBarrier(commandBuffer);
}

void HiZDrawSubrender::Render(const CommandBuffer& commandBuffer)
{
    // first-pass graphics pipeline to do the first indirectDraw
    auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    if (!gpuScene) return;

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

    descriptorSetGraphics.Push("UniformCamera", uniformCamera);
    gpuScene->PushDescriptors(descriptorSetGraphics);

    if (!descriptorSetGraphics.Update(PipelineGraphics)) return;
    PipelineGraphics.BindPipeline(commandBuffer);

    descriptorSetGraphics.BindDescriptor(commandBuffer, PipelineGraphics);

    gpuScene->cmdRender(commandBuffer);
}

void HiZDrawSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    // fisrt-pass compute pipeline to produce Hi-z
    uint32_t maxMipLevel = Image::GetMipLevels({Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y, 1});

    descriptorSetsCompute.resize(maxMipLevel + 1);

    const auto& hiz        = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("Hi-z"));
    const auto& depthToHiz = dynamic_cast<const Image*>(Graphics::Get()->GetAttachment("depthToHiz"));

    glm::uvec2 previousLevelDimensions, currentDimensions;
    uint32_t   mipLevel     = 0;
    previousLevelDimensions = {Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y};
    currentDimensions       = {Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y};

    while (mipLevel < maxMipLevel) {
        pushHandlers[mipLevel].Push("previousLevelDimensions", previousLevelDimensions);
        pushHandlers[mipLevel].Push("currentDimensions", currentDimensions);
        pushHandlers[mipLevel].Push("mipLevel", mipLevel);

        if (mipLevel == 0)
            descriptorSetsCompute[mipLevel].Push("depthBuffer", Graphics::Get()->GetAttachment("depthToHiz"));
        else
            descriptorSetsCompute[mipLevel].Push("depthBuffer", hiz);

        descriptorSetsCompute[mipLevel].Push("HiZ", HiDepths[mipLevel]);
        descriptorSetsCompute[mipLevel].Push("PushObject", pushHandlers[mipLevel]);
        if (!descriptorSetsCompute[mipLevel].Update(pipelineCompute2)) continue;

        pipelineCompute2.BindPipeline(commandBuffer);
        descriptorSetsCompute[mipLevel].BindDescriptor(commandBuffer, pipelineCompute2);
        pushHandlers[mipLevel].BindPush(commandBuffer, pipelineCompute2);

        pipelineCompute2.CmdRender(commandBuffer, glm::uvec2(currentDimensions.x, currentDimensions.y));

        hiz->AddHierarchicalDepth(commandBuffer,
                                  HiDepths[mipLevel]->GetImage(),
                                  {currentDimensions.x, currentDimensions.y, 0},
                                  VK_FORMAT_R32_SFLOAT,
                                  VK_IMAGE_LAYOUT_GENERAL,
                                  mipLevel,
                                  0);

        HiDepths[mipLevel]->Image2dPipelineBarrierComputeToCompute(commandBuffer);

        previousLevelDimensions = currentDimensions;
        currentDimensions.x     = currentDimensions.x == 1 ? 1 : currentDimensions.x / 2;
        currentDimensions.y     = currentDimensions.y == 1 ? 1 : currentDimensions.y / 2;
        mipLevel++;
    }

    hiz->ImageHierarchyZPipelineBarrierComputeToGraphic(commandBuffer);
}

void HiZDrawSubrender::RecreateHiDepths()
{
    auto       logicalDevice = Graphics::Get()->GetLogicalDevice();
    auto       graphicsQueue = logicalDevice->GetGraphicsQueue();
    glm::uvec2 depthsExtent  = glm::uvec2(Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y);

    if (HiDepths.empty() || HiDepths[0]->GetExtent().width != depthsExtent.x || HiDepths[0]->GetExtent().height != depthsExtent.y) {
        uint32_t maxMipLevel   = Image::GetMipLevels({Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y, 1});
        uint32_t mipLevelToDep = 0;

        Graphics::CheckVk(vkQueueWaitIdle(graphicsQueue));
        HiDepths.clear();

        while (mipLevelToDep < maxMipLevel) {
            // if (depthsExtent.x == 1 || depthsExtent.y == 1) break;
            HiDepths.push_back(
                std::make_unique<Image2d>(depthsExtent,
                                          VK_FORMAT_R32_SFLOAT,
                                          VK_IMAGE_LAYOUT_GENERAL,
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                          VK_FILTER_LINEAR,
                                          VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                          VK_SAMPLE_COUNT_1_BIT));
            depthsExtent.x /= 2;
            depthsExtent.y /= 2;
            mipLevelToDep++;
        }

        if (pushHandlers.size() != maxMipLevel) pushHandlers.resize(maxMipLevel);
    }
}
}   // namespace MapleLeaf