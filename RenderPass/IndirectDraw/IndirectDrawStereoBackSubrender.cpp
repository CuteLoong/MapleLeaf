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
    const auto& leftMaxHiZ  = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("LeftMaxHi-z"));
    const auto& RightMaxHiZ = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("RightMaxHi-z"));

    const auto& depth = dynamic_cast<const Image*>(Graphics::Get()->GetAttachment("backDepth"));

    uint32_t maxMipLevel = pushHandlers.size();

    descriptorSetComputeHiZMax.resize(maxMipLevel);

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
            descriptorSetComputeHiZMax[mipLevel].Push("LeftDepthBuffer", depth);
            descriptorSetComputeHiZMax[mipLevel].Push("RightDepthBuffer", depth);
        }
        else {
            descriptorSetComputeHiZMax[mipLevel].Push("LeftDepthBuffer", leftMaxHiZ);
            descriptorSetComputeHiZMax[mipLevel].Push("RightDepthBuffer", RightMaxHiZ);
        }

        descriptorSetComputeHiZMax[mipLevel].Push("LeftHiZ", maxHiDepthsLeft[mipLevel]);
        descriptorSetComputeHiZMax[mipLevel].Push("RightHiZ", maxHiDepthsRight[mipLevel]);

        descriptorSetComputeHiZMax[mipLevel].Push("PushObject", pushHandlers[mipLevel]);

        if (!descriptorSetComputeHiZMax[mipLevel].Update(pipelineComputeHiZMax)) continue;

        pipelineComputeHiZMax.BindPipeline(commandBuffer);
        descriptorSetComputeHiZMax[mipLevel].BindDescriptor(commandBuffer, pipelineComputeHiZMax);
        pushHandlers[mipLevel].BindPush(commandBuffer, pipelineComputeHiZMax);
        if (mipLevel == 0)
            pipelineComputeHiZMax.CmdRender(commandBuffer, glm::uvec2(currentDimensions.x * 2u, currentDimensions.y));
        else
            pipelineComputeHiZMax.CmdRender(commandBuffer, glm::uvec2(currentDimensions.x, currentDimensions.y));

        leftMaxHiZ->AddHierarchicalDepth(commandBuffer,
                                         maxHiDepthsLeft[mipLevel]->GetImage(),
                                         {currentDimensions.x, currentDimensions.y, 0},
                                         VK_FORMAT_R32_SFLOAT,
                                         VK_IMAGE_LAYOUT_GENERAL,
                                         mipLevel,
                                         0);

        RightMaxHiZ->AddHierarchicalDepth(commandBuffer,
                                          maxHiDepthsRight[mipLevel]->GetImage(),
                                          {currentDimensions.x, currentDimensions.y, 0},
                                          VK_FORMAT_R32_SFLOAT,
                                          VK_IMAGE_LAYOUT_GENERAL,
                                          mipLevel,
                                          0);

        maxHiDepthsLeft[mipLevel]->Image2dPipelineBarrierComputeToCompute(commandBuffer);
        maxHiDepthsRight[mipLevel]->Image2dPipelineBarrierComputeToCompute(commandBuffer);

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

    if (maxHiDepthsLeft.empty() || maxHiDepthsLeft[0]->GetExtent().width != depthsExtent.x ||
        maxHiDepthsLeft[0]->GetExtent().height != depthsExtent.y) {
        uint32_t maxMipLevel   = Image::GetMipLevels({depthsExtent.x, depthsExtent.y, 1});
        uint32_t mipLevelToDep = 0;

        maxHiDepthsLeft.clear();
        maxHiDepthsRight.clear();

        while (mipLevelToDep < maxMipLevel) {
            maxHiDepthsLeft.push_back(
                std::make_unique<Image2d>(glm::uvec2(depthsExtent.x, depthsExtent.y),
                                          VK_FORMAT_R32_SFLOAT,
                                          VK_IMAGE_LAYOUT_GENERAL,
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                          VK_FILTER_LINEAR,
                                          VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                          VK_SAMPLE_COUNT_1_BIT));
            maxHiDepthsRight.push_back(
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