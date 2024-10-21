#include "RadianceFilterSubrender.hpp"

#include "DescriptorHandler.hpp"
#include "Graphics.hpp"
#include "Pipeline.hpp"
#include "PipelineCompute.hpp"
#include "Scenes.hpp"

namespace MONO_Subrender {
RadianceFilterSubrender::RadianceFilterSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineRadianceFilter("Shader/InterpolationMono/RadianceFilter.comp", {}, false)
{}

void RadianceFilterSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    const auto& MultiLevelLighting     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelLighting"));
    const auto& MultiLevelPrevLighting = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelPrevLighting"));

    const auto& MultiLevelDepth     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelDepth"));
    const auto& MultiLevelPrevDepth = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelPrevDepth"));

    for (uint32_t i = 0; i < MultiLevelLighting->GetMipLevels(); i++)
        Image::CopyImage(commandBuffer, *MultiLevelLighting, *MultiLevelPrevLighting, i, i);

    for (uint32_t i = 0; i < MultiLevelDepth->GetMipLevels(); i++) Image::CopyImage(commandBuffer, *MultiLevelDepth, *MultiLevelPrevDepth, i, i);
}

void RadianceFilterSubrender::Render(const CommandBuffer& commandBuffer) {}

void RadianceFilterSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    const auto& lighting            = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("lighting"));
    const auto& depth               = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("depth"));
    const auto& RadianceFilterColor = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("RadianceFilterColor"));
    const auto& MultiLevelLighting  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelLighting"));
    const auto& MultiLevelDepth     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelDepth"));

    uint32_t maxMipLevel = Image::GetMipLevels({Devices::Get()->GetWindow()->GetSize().x, Devices::Get()->GetWindow()->GetSize().y, 1});

    descriptorSetRadianceFilter.resize(maxMipLevel);
    pushHandlers.resize(maxMipLevel);

    glm::uvec2 previousLevelDimensions, currentDimensions;
    uint32_t   mipLevel     = 0;
    previousLevelDimensions = Devices::Get()->GetWindow()->GetSize();
    currentDimensions       = Devices::Get()->GetWindow()->GetSize();

    while (mipLevel < maxMipLevel) {
        pushHandlers[mipLevel].Push("previousLevelDimensions", previousLevelDimensions);
        pushHandlers[mipLevel].Push("currentDimensions", currentDimensions);
        pushHandlers[mipLevel].Push("mipLevel", mipLevel);

        if (mipLevel == 0) {
            descriptorSetRadianceFilter[mipLevel].Push("radianceBuffer", lighting);
            descriptorSetRadianceFilter[mipLevel].Push("depthBuffer", depth);
        }
        else {
            descriptorSetRadianceFilter[mipLevel].Push("radianceBuffer", MultiLevelLighting, mipLevel - 1, std::nullopt, std::nullopt);
            descriptorSetRadianceFilter[mipLevel].Push("depthBuffer", MultiLevelDepth, mipLevel - 1, std::nullopt, std::nullopt);
        }
        descriptorSetRadianceFilter[mipLevel].Push("PushObject", pushHandlers[mipLevel]);
        descriptorSetRadianceFilter[mipLevel].Push("RadianceFilter", MultiLevelLighting, mipLevel, std::nullopt, std::nullopt);
        descriptorSetRadianceFilter[mipLevel].Push("DepthFilter", MultiLevelDepth, mipLevel, std::nullopt, std::nullopt);

        if (!descriptorSetRadianceFilter[mipLevel].Update(pipelineRadianceFilter)) return;
        pipelineRadianceFilter.BindPipeline(commandBuffer);
        descriptorSetRadianceFilter[mipLevel].BindDescriptor(commandBuffer, pipelineRadianceFilter);
        pushHandlers[mipLevel].BindPush(commandBuffer, pipelineRadianceFilter);

        pipelineRadianceFilter.CmdRender(commandBuffer, {currentDimensions.x, currentDimensions.y});
        MultiLevelLighting->Image2dPipelineBarrierComputeToCompute(commandBuffer, mipLevel);
        MultiLevelDepth->Image2dPipelineBarrierComputeToCompute(commandBuffer, mipLevel);

        previousLevelDimensions = currentDimensions;
        currentDimensions.x     = currentDimensions.x == 1 ? 1 : currentDimensions.x / 2;
        currentDimensions.y     = currentDimensions.y == 1 ? 1 : currentDimensions.y / 2;
        mipLevel++;
    }
}
}   // namespace MONO_Subrender