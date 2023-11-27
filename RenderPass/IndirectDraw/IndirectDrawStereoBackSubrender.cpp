#include "IndirectDrawStereoBackSubrender.hpp"

#include "Scenes.hpp"

namespace MapleLeaf {
IndirectDrawStereoBackSubrender::IndirectDrawStereoBackSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage,
               {"Shader/GPUDriven/DefaultStereoBack.vert", "Shader/GPUDriven/MultiviewBack.geom", "Shader/GPUDriven/DefaultStereoBack.frag"},
               {Vertex3D::GetVertexInput()}, {}, PipelineGraphics::Mode::Stereo, PipelineGraphics::Depth::ReadWrite,
               VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
    , pipelineComputeHiZMax("Shader/GPUDriven/HierarchicalDepthStereo-Max.comp")
{
    RecreateHiDepths();
}

void IndirectDrawStereoBackSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void IndirectDrawStereoBackSubrender::Render(const CommandBuffer& commandBuffer)
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

void IndirectDrawStereoBackSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    const auto& maxHiZ = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("MaxHi-z"));

    const auto& depth = dynamic_cast<const Image*>(Graphics::Get()->GetAttachment("backDepth"));

    uint32_t maxMipLevel = pushHandlers.size();

    descriptorSetComputeHiZMax.resize(maxMipLevel);

    glm::uvec2 previousLevelDimensions, currentDimensions;
    uint32_t   mipLevel     = 0;
    previousLevelDimensions = Devices::Get()->GetWindow()->GetStereoSize();
    currentDimensions       = Devices::Get()->GetWindow()->GetStereoSize();

    while (mipLevel < maxMipLevel) {
        pushHandlers[mipLevel].Push("previousLevelMonoDimensions", previousLevelDimensions);
        pushHandlers[mipLevel].Push("currentMonoDimensions", currentDimensions);
        pushHandlers[mipLevel].Push("mipLevel", mipLevel);

        if (mipLevel == 0)
            descriptorSetComputeHiZMax[mipLevel].Push("depthBuffer", depth);
        else
            descriptorSetComputeHiZMax[mipLevel].Push("depthBuffer", maxHiZ);


        descriptorSetComputeHiZMax[mipLevel].Push("HiZ", maxHiDepths[mipLevel]);

        descriptorSetComputeHiZMax[mipLevel].Push("PushObject", pushHandlers[mipLevel]);

        if (!descriptorSetComputeHiZMax[mipLevel].Update(pipelineComputeHiZMax)) continue;

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

        maxHiDepths[mipLevel]->Image2dPipelineBarrierComputeToCompute(commandBuffer);

        previousLevelDimensions = currentDimensions;
        currentDimensions.x     = currentDimensions.x == 1 ? 1 : currentDimensions.x / 2u;
        currentDimensions.y     = currentDimensions.y == 1 ? 1 : currentDimensions.y / 2u;
        mipLevel++;
    }
}


void IndirectDrawStereoBackSubrender::RecreateHiDepths()
{
    auto       logicalDevice = Graphics::Get()->GetLogicalDevice();
    auto       graphicsQueue = logicalDevice->GetGraphicsQueue();
    glm::uvec2 depthsExtent  = glm::uvec2(Devices::Get()->GetWindow()->GetStereoSize());

    if (maxHiDepths.empty() || maxHiDepths[0]->GetExtent().width != depthsExtent.x * 2u || maxHiDepths[0]->GetExtent().height != depthsExtent.y) {
        uint32_t maxMipLevel   = Image::GetMipLevels({depthsExtent.x, depthsExtent.y, 1}) - 1;
        uint32_t mipLevelToDep = 0;

        Graphics::CheckVk(vkQueueWaitIdle(graphicsQueue));
        maxHiDepths.clear();

        while (mipLevelToDep < maxMipLevel) {
            // if (depthsExtent.x == 1 || depthsExtent.y == 1) break;
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