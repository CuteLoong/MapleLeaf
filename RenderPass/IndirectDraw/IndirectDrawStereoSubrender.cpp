#include "IndirectDrawStereoSubrender.hpp"

#include "Scenes.hpp"

namespace MapleLeaf {
IndirectDrawStereoSubrender::IndirectDrawStereoSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , compute("Shader/GPUDriven/CullingStereo.comp")
    , pipeline(pipelineStage, {"Shader/GPUDriven/DefaultStereo.vert", "Shader/GPUDriven/Multiview.geom", "Shader/GPUDriven/DefaultStereo.frag"},
               {Vertex3D::GetVertexInput()}, {}, PipelineGraphics::Mode::StereoMRT)
    , pipelineComputeHiZMin("Shader/GPUDriven/HierarchicalDepthStereo-Min.comp")
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
    const auto& leftMinHiZ  = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("LeftMinHi-z"));
    const auto& RightMinHiZ = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("RightMinHi-z"));

    const auto& depth = dynamic_cast<const Image*>(Graphics::Get()->GetAttachment("depth"));

    uint32_t maxMipLevel = pushHandlers.size();

    descriptorSetComputeHiZMin.resize(maxMipLevel);

    glm::uvec2 previousLevelDimensions, currentDimensions;
    uint32_t   mipLevel     = 0;
    previousLevelDimensions = Devices::Get()->GetWindow()->GetStereoSize();
    currentDimensions       = Devices::Get()->GetWindow()->GetStereoSize();

    while (mipLevel < maxMipLevel) {
        if (mipLevel == 0) {
            pushHandlers[mipLevel].Push("previousLevelDimensions", Devices::Get()->GetWindow()->GetSize());
            pushHandlers[mipLevel].Push("currentDimensions", Devices::Get()->GetWindow()->GetSize());
        }
        else {
            pushHandlers[mipLevel].Push("previousLevelDimensions", previousLevelDimensions);
            pushHandlers[mipLevel].Push("currentDimensions", currentDimensions);
        }
        pushHandlers[mipLevel].Push("mipLevel", mipLevel);

        if (mipLevel == 0) {
            descriptorSetComputeHiZMin[mipLevel].Push("LeftDepthBuffer", depth);
            descriptorSetComputeHiZMin[mipLevel].Push("RightDepthBuffer", depth);
        }
        else {
            descriptorSetComputeHiZMin[mipLevel].Push("LeftDepthBuffer", leftMinHiZ);
            descriptorSetComputeHiZMin[mipLevel].Push("RightDepthBuffer", RightMinHiZ);
        }

        descriptorSetComputeHiZMin[mipLevel].Push("LeftHiZ", minHiDepthsLeft[mipLevel]);
        descriptorSetComputeHiZMin[mipLevel].Push("RightHiZ", minHiDepthsRight[mipLevel]);

        descriptorSetComputeHiZMin[mipLevel].Push("PushObject", pushHandlers[mipLevel]);

        if (!descriptorSetComputeHiZMin[mipLevel].Update(pipelineComputeHiZMin)) continue;

        pipelineComputeHiZMin.BindPipeline(commandBuffer);
        descriptorSetComputeHiZMin[mipLevel].BindDescriptor(commandBuffer, pipelineComputeHiZMin);
        pushHandlers[mipLevel].BindPush(commandBuffer, pipelineComputeHiZMin);
        if (mipLevel == 0)
            pipelineComputeHiZMin.CmdRender(commandBuffer, glm::uvec2(currentDimensions.x * 2u, currentDimensions.y));
        else
            pipelineComputeHiZMin.CmdRender(commandBuffer, glm::uvec2(currentDimensions.x, currentDimensions.y));

        leftMinHiZ->AddHierarchicalDepth(commandBuffer,
                                         minHiDepthsLeft[mipLevel]->GetImage(),
                                         {currentDimensions.x, currentDimensions.y, 0},
                                         VK_FORMAT_R32_SFLOAT,
                                         VK_IMAGE_LAYOUT_GENERAL,
                                         mipLevel,
                                         0);

        RightMinHiZ->AddHierarchicalDepth(commandBuffer,
                                          minHiDepthsRight[mipLevel]->GetImage(),
                                          {currentDimensions.x, currentDimensions.y, 0},
                                          VK_FORMAT_R32_SFLOAT,
                                          VK_IMAGE_LAYOUT_GENERAL,
                                          mipLevel,
                                          0);

        minHiDepthsLeft[mipLevel]->Image2dPipelineBarrierComputeToCompute(commandBuffer);
        minHiDepthsRight[mipLevel]->Image2dPipelineBarrierComputeToCompute(commandBuffer);

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

    if (minHiDepthsLeft.empty() || minHiDepthsLeft[0]->GetExtent().width != depthsExtent.x ||
        minHiDepthsLeft[0]->GetExtent().height != depthsExtent.y) {
        uint32_t maxMipLevel   = Image::GetMipLevels({depthsExtent.x, depthsExtent.y, 1});
        uint32_t mipLevelToDep = 0;

        minHiDepthsLeft.clear();
        minHiDepthsRight.clear();

        while (mipLevelToDep < maxMipLevel) {
            minHiDepthsLeft.push_back(
                std::make_unique<Image2d>(glm::uvec2(depthsExtent.x, depthsExtent.y),
                                          VK_FORMAT_R32_SFLOAT,
                                          VK_IMAGE_LAYOUT_GENERAL,
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                          VK_FILTER_LINEAR,
                                          VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                          VK_SAMPLE_COUNT_1_BIT));
            minHiDepthsRight.push_back(
                std::make_unique<Image2d>(glm::uvec2(depthsExtent.x, depthsExtent.y),
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