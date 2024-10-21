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
    glm::uvec2 previousLevelDimensions = Devices::Get()->GetWindow()->GetSize();
    glm::uvec2 currentDimensions       = Devices::Get()->GetWindow()->GetSize();

    const auto& MultiLevelLighting     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelLighting"));
    const auto& MultiLevelPrevLighting = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelPrevLighting"));


    if (mipImage.empty()) {
        // filteredRadiance = std::make_unique<Image2d>(Graphics::Get()->GetNonRTAttachmentSize(),
        //                                              VK_FORMAT_R16G16B16A16_SFLOAT,
        //                                              VK_IMAGE_LAYOUT_GENERAL,
        //                                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        //                                              VK_FILTER_LINEAR,
        //                                              VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        //                                              VK_SAMPLE_COUNT_1_BIT,
        //                                              true,
        //                                              true);
        mipImage.resize(MultiLevelLighting->GetMipLevels());
        for (uint32_t i = 0; i < mipImage.size(); i++) {
            mipImage[i]         = std::move(std::make_unique<Image2d>(currentDimensions,
                                                              VK_FORMAT_R16G16B16A16_SFLOAT,
                                                              VK_IMAGE_LAYOUT_GENERAL,
                                                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                                                              VK_FILTER_LINEAR,
                                                              VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                              VK_SAMPLE_COUNT_1_BIT));
            currentDimensions.x = currentDimensions.x == 1 ? 1 : currentDimensions.x / 2;
            currentDimensions.y = currentDimensions.y == 1 ? 1 : currentDimensions.y / 2;
        }
    }
    else {
        for (uint32_t i = 0; i < mipImage.size(); i++) Image::CopyImage(commandBuffer, *MultiLevelLighting, *MultiLevelPrevLighting, i, i);
    }
}

void RadianceFilterSubrender::Render(const CommandBuffer& commandBuffer) {}

void RadianceFilterSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    const auto& lighting = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("lighting"));
    const auto& depth    = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("depth"));
    // const auto& normal              = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("normal"));
    // const auto& position            = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("position"));
    const auto& RadianceFilterColor = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("RadianceFilterColor"));
    const auto& MultiLevelLighting  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelLighting"));

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

        if (mipLevel == 0)
            descriptorSetRadianceFilter[mipLevel].Push("radianceBuffer", lighting);
        else {
            descriptorSetRadianceFilter[mipLevel].Push("radianceBuffer", MultiLevelLighting, mipLevel - 1, std::nullopt, std::nullopt);
        }
        descriptorSetRadianceFilter[mipLevel].Push("PushObject", pushHandlers[mipLevel]);
        descriptorSetRadianceFilter[mipLevel].Push("RadianceFilterColor", MultiLevelLighting, mipLevel, std::nullopt, std::nullopt);

        if (!descriptorSetRadianceFilter[mipLevel].Update(pipelineRadianceFilter)) return;
        pipelineRadianceFilter.BindPipeline(commandBuffer);
        descriptorSetRadianceFilter[mipLevel].BindDescriptor(commandBuffer, pipelineRadianceFilter);
        pushHandlers[mipLevel].BindPush(commandBuffer, pipelineRadianceFilter);

        pipelineRadianceFilter.CmdRender(commandBuffer, {currentDimensions.x, currentDimensions.y});
        // mipImage[mipLevel]->Image2dPipelineBarrierComputeToCompute(commandBuffer);

        // MultiLevelLighting->CopyImage2d(commandBuffer, *mipImage[mipLevel], mipLevel);
        // filteredRadiance->CopyImage2d(commandBuffer, *mipImage[mipLevel], mipLevel);   // copy image2d to mipLevel

        previousLevelDimensions = currentDimensions;
        currentDimensions.x     = currentDimensions.x == 1 ? 1 : currentDimensions.x / 2;
        currentDimensions.y     = currentDimensions.y == 1 ? 1 : currentDimensions.y / 2;
        mipLevel++;
    }

    // auto camera = Scenes::Get()->GetScene()->GetCamera();
    // camera->PushUniforms(uniformCameraRadianceFilter);

    // descriptorSetRadianceFilter.Push("UniformCamera", uniformCameraRadianceFilter);
    // descriptorSetRadianceFilter.Push("radiance", lighting);
    // descriptorSetRadianceFilter.Push("depth", depth);
    // descriptorSetRadianceFilter.Push("normal", normal);
    // descriptorSetRadianceFilter.Push("position", position);
    // descriptorSetRadianceFilter.Push("RadianceFilterColor", RadianceFilterColor);

    // if (!descriptorSetRadianceFilter.Update(pipelineRadianceFilter)) return;

    // pipelineRadianceFilter.BindPipeline(commandBuffer);
    // descriptorSetRadianceFilter.BindDescriptor(commandBuffer, pipelineRadianceFilter);
    // pipelineRadianceFilter.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());
}
}   // namespace MONO_Subrender