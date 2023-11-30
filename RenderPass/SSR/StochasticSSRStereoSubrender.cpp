#include "StochasticSSRStereoSubrender.hpp"

#include "Graphics.hpp"
#include "Image2d.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
StochasticSSRStereoSubrender::StochasticSSRStereoSubrender(const Pipeline::Stage& pipelineStage, SSRData ssrData)
    : Subrender(pipelineStage)
    , ssrData(ssrData)
    , pipelineCompute("Shader/SSR/StochasticSSR.comp", {}, false)
    , brdf(Resources::Get()->GetThreadPool().Enqueue(ComputeBRDF, 512))
    , blueNoise(Resources::Get()->GetThreadPool().Enqueue(LoadBlueNoise))
{
    haltonSampler = HaltonSamplePattern::Create(4);
}

void StochasticSSRStereoSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void StochasticSSRStereoSubrender::Render(const CommandBuffer& commandBuffer) {}

void StochasticSSRStereoSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    const auto& SSRHitsMap = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("SSRHitsMap"));
    const auto& SSRMask    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("SSRMask"));

    const auto& minHiZ = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("MinHi-z"));
    const auto& maxHiZ = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("MaxHi-z"));

    glm::vec2 noiseScale = static_cast<glm::vec2>(Devices::Get()->GetWindow()->GetSize()) / static_cast<glm::vec2>((*blueNoise)->GetSize());

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

    uniformSSRData.Push("maxRayLength", ssrData.maxRayLength);
    uniformSSRData.Push("maxSteps", ssrData.maxSteps);
    uniformSSRData.Push("zThickness", ssrData.zThickness);
    uniformSSRData.Push("brdfBias", ssrData.brdfBias);
    uniformSSRData.Push("hiZStartLevel", std::min(static_cast<float>(minHiZ->GetMipLevels()), 2.0f));
    uniformSSRData.Push("hiZMaxLevel", static_cast<float>(minHiZ->GetMipLevels()) - 1.0f);
    uniformSSRData.Push("noiseScale", noiseScale);

    uniformJitterData.Push("jitter", haltonSampler->next());

    descriptorSet.Push("UniformSSRData", uniformSSRData);
    descriptorSet.Push("UniformJitterData", uniformJitterData);
    descriptorSet.Push("UniformCamera", uniformCamera);
    descriptorSet.Push("inDepth", Graphics::Get()->GetAttachment("depth"));
    descriptorSet.Push("inNormal", Graphics::Get()->GetAttachment("normal"));
    descriptorSet.Push("inMaterial", Graphics::Get()->GetAttachment("material"));
    descriptorSet.Push("inMinHiZ", Graphics::Get()->GetNonRTAttachment("MinHi-z"));
    descriptorSet.Push("inMaxHiZ", Graphics::Get()->GetNonRTAttachment("MaxHi-z"));
    descriptorSet.Push("preIntegratedDFG", *brdf);
    descriptorSet.Push("blueNoise", *blueNoise);
    descriptorSet.Push("SSRHitsMap", SSRHitsMap);
    descriptorSet.Push("SSRMask", SSRMask);

    if (!descriptorSet.Update(pipelineCompute)) return;

    pipelineCompute.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipelineCompute);

    pipelineCompute.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

    SSRHitsMap->Image2dPipelineBarrierComputeToGraphic(commandBuffer);
}

std::unique_ptr<Image2d> StochasticSSRStereoSubrender::ComputeBRDF(uint32_t size)
{
    auto brdfImage = std::make_unique<Image2d>(glm::uvec2(size), VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_GENERAL);

    CommandBuffer   commandBuffer(true, VK_QUEUE_COMPUTE_BIT);
    PipelineCompute compute("Shader/Skybox/PreIntegrationDFG.comp");

    // Bind the pipeline.
    compute.BindPipeline(commandBuffer);

    // Updates descriptors.
    DescriptorsHandler descriptorSet(compute);
    descriptorSet.Push("preIntegratedDFG", brdfImage.get());
    descriptorSet.Update(compute);

    // Runs the compute pipeline.
    descriptorSet.BindDescriptor(commandBuffer, compute);
    compute.CmdRender(commandBuffer, brdfImage->GetSize());
    commandBuffer.SubmitIdle();

    return brdfImage;
}

std::shared_ptr<Image2d> StochasticSSRStereoSubrender::LoadBlueNoise()
{
    auto blueNoiseImage = Image2d::Create("NoiseImage/BlueNoise.tga", VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, false, false);

    return blueNoiseImage;
}
}   // namespace MapleLeaf