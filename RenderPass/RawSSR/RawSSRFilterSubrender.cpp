#include "RawSSRFilterSubrender.hpp"

#include "Graphics.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
RawSSRFilterSubrender::RawSSRFilterSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineCompute("Shader/Raw_SSR/SSRStereoSpatialFilter.comp", {}, false)
    , pipelineTemporalFilter("Shader/SSR/SSRStereoTemporalFilter.comp", {}, false)
    , blueNoise(Resources::Get()->GetThreadPool().Enqueue(LoadBlueNoise))
{
    haltonSampler = HaltonSamplePattern::Create(4);
}

void RawSSRFilterSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    const auto& SSRHitsMap    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("SSRHitsMap"));
    const auto& SSRMask       = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("SSRMask"));
    const auto& ReflectionMap = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("ReflectionMap"));

    glm::vec2 noiseScale = static_cast<glm::vec2>(Devices::Get()->GetWindow()->GetSize()) / static_cast<glm::vec2>((*blueNoise)->GetSize());

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

    uniformFilterData.Push("noiseScale", noiseScale);
    uniformJitterData.Push("jitter", haltonSampler->next());

    descriptorSet.Push("UniformFilterData", uniformFilterData);
    descriptorSet.Push("UniformJitterData", uniformJitterData);
    descriptorSet.Push("UniformCamera", uniformCamera);
    descriptorSet.Push("inDepth", Graphics::Get()->GetAttachment("depth"));
    descriptorSet.Push("inNormal", Graphics::Get()->GetAttachment("normal"));
    descriptorSet.Push("inDiffuse", Graphics::Get()->GetAttachment("diffuse"));
    descriptorSet.Push("inMaterial", Graphics::Get()->GetAttachment("material"));
    descriptorSet.Push("LightingMap", Graphics::Get()->GetAttachment("lighting"));
    descriptorSet.Push("SSRHitsMap", SSRHitsMap);
    descriptorSet.Push("SSRHitsMask", SSRMask);
    descriptorSet.Push("blueNoise", *blueNoise);
    descriptorSet.Push("ReflectionColorMap", ReflectionMap);

    if (!descriptorSet.Update(pipelineCompute)) return;

    pipelineCompute.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipelineCompute);

    pipelineCompute.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());
    ReflectionMap->Image2dPipelineBarrierComputeToCompute(commandBuffer);
}

void RawSSRFilterSubrender::Render(const CommandBuffer& commandBuffer) {}

void RawSSRFilterSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    // Temporal Filter
    const auto& PrevSSRColor     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("PrevSSRColor"));
    const auto& TemporalSSRColor = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("TemporalSSRColor"));
    const auto& SSRHitsMap       = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("SSRHitsMap"));
    const auto& ReflectionMap    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("ReflectionMap"));

    SSRHitsMap->Image2dPipelineBarrierComputeToCompute(commandBuffer);
    ReflectionMap->Image2dPipelineBarrierComputeToCompute(commandBuffer);

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCameraTemporalFilter);

    descriptorSetTemporalFilter.Push("UniformCamera", uniformCameraTemporalFilter);
    descriptorSetTemporalFilter.Push("inDepth", Graphics::Get()->GetAttachment("depth"));
    descriptorSetTemporalFilter.Push("SSRHitsMap", SSRHitsMap);
    descriptorSetTemporalFilter.Push("PrevSSRColor", PrevSSRColor);
    descriptorSetTemporalFilter.Push("CurSSRColor", ReflectionMap);
    descriptorSetTemporalFilter.Push("TemporalSSRColor", TemporalSSRColor);

    if (!descriptorSetTemporalFilter.Update(pipelineTemporalFilter)) return;

    pipelineTemporalFilter.BindPipeline(commandBuffer);

    descriptorSetTemporalFilter.BindDescriptor(commandBuffer, pipelineTemporalFilter);

    pipelineTemporalFilter.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

    PrevSSRColor->CopyImage2d(commandBuffer, *TemporalSSRColor);
}

std::shared_ptr<Image2d> RawSSRFilterSubrender::LoadBlueNoise()
{
    auto blueNoiseImage = Image2d::Create("NoiseImage/BlueNoise.tga", VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, false, false);

    return blueNoiseImage;
}
}   // namespace MapleLeaf