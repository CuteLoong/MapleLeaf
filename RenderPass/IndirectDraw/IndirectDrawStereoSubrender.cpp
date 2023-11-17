#include "IndirectDrawStereoSubrender.hpp"

#include "Scenes.hpp"

namespace MapleLeaf {
IndirectDrawStereoSubrender::IndirectDrawStereoSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , compute("Shader/GPUDriven/CullingStereo.comp")
    , pipeline(pipelineStage, {"Shader/GPUDriven/DefaultStereo.vert", "Shader/GPUDriven/Multiview.geom", "Shader/GPUDriven/DefaultStereo.frag"},
               {Vertex3D::GetVertexInput()}, {}, PipelineGraphics::Mode::StereoMRT)
    , pipelineComputeHiZMin("Shader/GPUDriven/HierarchicalDepthStereo-Min.comp")
    , pipelineComputeHiZMax("Shader/GPUDriven/HierarchicalDepthStereo-Max.comp")
{
    RecreateHiDepths();
}

void IndirectDrawStereoSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    const auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    if (!gpuScene) return;
    if (!gpuScene->GetIndirectBuffer()) return;

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCameraCompute);

    uint32_t instanceCount = gpuScene->GetInstanceCount();

    pushHandler.Push("instanceCount", instanceCount);

    descriptorSetCompute.Push("InstanceDatas", gpuScene->GetInstanceDatasHandler());
    descriptorSetCompute.Push("DrawCommandBuffer", gpuScene->GetIndirectBuffer());
    descriptorSetCompute.Push("UniformCamera", uniformCameraCompute);
    descriptorSetCompute.Push("PushObject", pushHandler);

    if (!descriptorSetCompute.Update(compute)) return;
    compute.BindPipeline(commandBuffer);
    descriptorSetCompute.BindDescriptor(commandBuffer, compute);
    pushHandler.BindPush(commandBuffer, compute);
    compute.CmdRender(commandBuffer, glm::uvec2(instanceCount, 1));

    gpuScene->GetIndirectBuffer()->IndirectBufferPipelineBarrier(commandBuffer);
}

void IndirectDrawStereoSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    if (!gpuScene) return;

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

    descriptorSetGraphics.Push("UniformCamera", uniformCamera);
    gpuScene->PushDescriptors(descriptorSetGraphics);

    if (!descriptorSetGraphics.Update(pipeline)) return;
    pipeline.BindPipeline(commandBuffer);

    descriptorSetGraphics.BindDescriptor(commandBuffer, pipeline);

    gpuScene->cmdRender(commandBuffer);
}

void IndirectDrawStereoSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    const auto& minHiZ = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("MinHi-z"));
    const auto& maxHiZ = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("MaxHi-z"));

    const auto& depth = dynamic_cast<const Image*>(Graphics::Get()->GetAttachment("depth"));

    uint32_t maxMipLevel = pushHandlers.size();

    descriptorSetComputeHiZMin.resize(maxMipLevel);
    descriptorSetComputeHiZMax.resize(maxMipLevel);

    glm::uvec2 previousLevelDimensions, currentDimensions;
    uint32_t   mipLevel     = 0;
    previousLevelDimensions = Devices::Get()->GetWindow()->GetStereoSize();
    currentDimensions       = Devices::Get()->GetWindow()->GetStereoSize();

    while (mipLevel < maxMipLevel) {
        pushHandlers[mipLevel].Push("previousLevelMonoDimensions", previousLevelDimensions);
        pushHandlers[mipLevel].Push("currentMonoDimensions", currentDimensions);
        pushHandlers[mipLevel].Push("mipLevel", mipLevel);

        if (mipLevel == 0) {
            descriptorSetComputeHiZMin[mipLevel].Push("depthBuffer", depth);
            descriptorSetComputeHiZMax[mipLevel].Push("depthBuffer", depth);
        }
        else {
            descriptorSetComputeHiZMin[mipLevel].Push("depthBuffer", minHiZ);
            descriptorSetComputeHiZMax[mipLevel].Push("depthBuffer", maxHiZ);
        }

        descriptorSetComputeHiZMin[mipLevel].Push("HiZ", minHiDepths[mipLevel]);
        descriptorSetComputeHiZMax[mipLevel].Push("HiZ", maxHiDepths[mipLevel]);

        descriptorSetComputeHiZMin[mipLevel].Push("PushObject", pushHandlers[mipLevel]);
        descriptorSetComputeHiZMax[mipLevel].Push("PushObject", pushHandlers[mipLevel]);

        const bool updateMin = descriptorSetComputeHiZMin[mipLevel].Update(pipelineComputeHiZMin);
        const bool updateMax = descriptorSetComputeHiZMax[mipLevel].Update(pipelineComputeHiZMax);
        if (!updateMax || !updateMin) continue;

        pipelineComputeHiZMin.BindPipeline(commandBuffer);
        descriptorSetComputeHiZMin[mipLevel].BindDescriptor(commandBuffer, pipelineComputeHiZMin);
        pushHandlers[mipLevel].BindPush(commandBuffer, pipelineComputeHiZMin);
        pipelineComputeHiZMin.CmdRender(commandBuffer, glm::uvec2(currentDimensions.x * 2u, currentDimensions.y));

        minHiZ->AddHierarchicalDepth(commandBuffer,
                                     minHiDepths[mipLevel]->GetImage(),
                                     {currentDimensions.x * 2u, currentDimensions.y, 0},
                                     VK_FORMAT_R32_SFLOAT,
                                     VK_IMAGE_LAYOUT_GENERAL,
                                     mipLevel,
                                     0);

        pipelineComputeHiZMax.BindPipeline(commandBuffer);
        descriptorSetComputeHiZMax[mipLevel].BindDescriptor(commandBuffer, pipelineComputeHiZMax);
        pushHandlers[mipLevel].BindPush(commandBuffer, pipelineComputeHiZMax);
        pipelineComputeHiZMax.CmdRender(commandBuffer, glm::uvec2(currentDimensions.x * 2u, currentDimensions.y));

        maxHiZ->AddHierarchicalDepth(commandBuffer,
                                     maxHiDepths[mipLevel]->GetImage(),
                                     {currentDimensions.x * 2u, currentDimensions.y, 0},
                                     VK_FORMAT_R32_SFLOAT,
                                     VK_IMAGE_LAYOUT_GENERAL,
                                     mipLevel,
                                     0);

        minHiDepths[mipLevel]->Image2dPipelineBarrierComputeToCompute(commandBuffer);
        maxHiDepths[mipLevel]->Image2dPipelineBarrierComputeToCompute(commandBuffer);

        previousLevelDimensions = currentDimensions;
        currentDimensions.x     = currentDimensions.x == 1 ? 1 : currentDimensions.x / 2u;
        currentDimensions.y     = currentDimensions.y == 1 ? 1 : currentDimensions.y / 2u;
        mipLevel++;
    }
}

void IndirectDrawStereoSubrender::RecreateHiDepths()
{
    auto       logicalDevice = Graphics::Get()->GetLogicalDevice();
    auto       graphicsQueue = logicalDevice->GetGraphicsQueue();
    glm::uvec2 depthsExtent  = glm::uvec2(Devices::Get()->GetWindow()->GetStereoSize());

    if (minHiDepths.empty() || minHiDepths[0]->GetExtent().width != depthsExtent.x * 2u || minHiDepths[0]->GetExtent().height != depthsExtent.y) {
        uint32_t maxMipLevel   = Image::GetMipLevels({depthsExtent.x, depthsExtent.y, 1}) - 1;
        uint32_t mipLevelToDep = 0;

        Graphics::CheckVk(vkQueueWaitIdle(graphicsQueue));
        minHiDepths.clear();
        maxHiDepths.clear();

        while (mipLevelToDep < maxMipLevel) {
            // if (depthsExtent.x == 1 || depthsExtent.y == 1) break;
            minHiDepths.push_back(
                std::make_unique<Image2d>(glm::uvec2(depthsExtent.x * 2u, depthsExtent.y),
                                          VK_FORMAT_R32_SFLOAT,
                                          VK_IMAGE_LAYOUT_GENERAL,
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                          VK_FILTER_LINEAR,
                                          VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                          VK_SAMPLE_COUNT_1_BIT));
            maxHiDepths.push_back(
                std::make_unique<Image2d>(glm::uvec2(depthsExtent.x * 2u, depthsExtent.y),
                                          VK_FORMAT_R32_SFLOAT,
                                          VK_IMAGE_LAYOUT_GENERAL,
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                          VK_FILTER_LINEAR,
                                          VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                          VK_SAMPLE_COUNT_1_BIT));
            depthsExtent.x = depthsExtent.x == 1 ? 1 : depthsExtent.x / 2u;
            depthsExtent.y = depthsExtent.y == 1 ? 1 : depthsExtent.y / 2u;
            mipLevelToDep++;
        }

        if (pushHandlers.size() != maxMipLevel) pushHandlers.resize(maxMipLevel);
    }
}
}   // namespace MapleLeaf