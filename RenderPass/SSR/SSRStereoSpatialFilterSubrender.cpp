#include "SSRStereoSpatialFilterSubrender.hpp"

#include "Graphics.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
SSRStereoSpatialFilterSubrender::SSRStereoSpatialFilterSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineCompute("Shader/SSR/SSRStereoSpatialFilter.comp", {}, false)
    , blueNoise(Resources::Get()->GetThreadPool().Enqueue(LoadBlueNoise))
{
    haltonSampler = HaltonSamplePattern::Create(4);
}

void SSRStereoSpatialFilterSubrender::PreRender(const CommandBuffer& commandBuffer)
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
}

void SSRStereoSpatialFilterSubrender::Render(const CommandBuffer& commandBuffer) {}

void SSRStereoSpatialFilterSubrender::PostRender(const CommandBuffer& commandBuffer) {}

std::shared_ptr<Image2d> SSRStereoSpatialFilterSubrender::LoadBlueNoise()
{
    auto blueNoiseImage = Image2d::Create("NoiseImage/BlueNoise.tga", VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, false, false);

    return blueNoiseImage;
}
}   // namespace MapleLeaf