#include "StochasticSSRStereoSubrender.hpp"

#include "Graphics.hpp"
#include "Image2d.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
StochasticSSRStereoSubrender::StochasticSSRStereoSubrender(const Pipeline::Stage& pipelineStage, SSRData ssrData)
    : Subrender(pipelineStage)
    , ssrData(ssrData)
    , pipelineCompute("Shader/SSR/StochasticSSR.comp", {}, false)
    , pipelineReprojection("Shader/SSR/ReprojectionStereo.comp", {}, false)
    , blueNoise(Resources::Get()->GetThreadPool().Enqueue(LoadBlueNoise))
{
    haltonSampler = HaltonSamplePattern::Create(4);
}

void StochasticSSRStereoSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    const auto& SSRHitsMap                     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("SSRHitsMap"));
    const auto& glossyMV                       = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("GlossyMV"));
    const auto& SSRMask                        = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("SSRMask"));
    const auto& ReprojectionReflectionColorMap = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("ReprojectionReflectionColorMap"));

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformReprojectionCamera);

    descriptorSetReprojection.Push("UniformCamera", uniformReprojectionCamera);
    descriptorSetReprojection.Push("inDepth", Graphics::Get()->GetAttachment("depth"));
    descriptorSetReprojection.Push("inNormal", Graphics::Get()->GetAttachment("normal"));
    descriptorSet.Push("inPosition", Graphics::Get()->GetAttachment("position"));
    descriptorSetReprojection.Push("inDiffuse", Graphics::Get()->GetAttachment("diffuse"));
    descriptorSetReprojection.Push("inMaterial", Graphics::Get()->GetAttachment("material"));
    descriptorSetReprojection.Push("LightingMap", Graphics::Get()->GetAttachment("lighting"));
    descriptorSetReprojection.Push("SSRHitsMap", SSRHitsMap);
    descriptorSetReprojection.Push("SSRHitsMask", SSRMask);
    descriptorSetReprojection.Push("GlossyMV", glossyMV);
    descriptorSetReprojection.Push("ReprojectionReflectionColorMap", ReprojectionReflectionColorMap);

    if (!descriptorSetReprojection.Update(pipelineReprojection)) return;

    pipelineReprojection.BindPipeline(commandBuffer);

    descriptorSetReprojection.BindDescriptor(commandBuffer, pipelineReprojection);

    pipelineReprojection.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

    ReprojectionReflectionColorMap->Image2dPipelineBarrierComputeToCompute(commandBuffer);
}

void StochasticSSRStereoSubrender::Render(const CommandBuffer& commandBuffer) {}

void StochasticSSRStereoSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    const auto& SSRHitsMap = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("SSRHitsMap"));
    const auto& SSRMask    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("SSRMask"));
    const auto& glossyMV   = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("GlossyMV"));
    // const auto& debugMask  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("DebugMask"));

    const auto& LeftMinHiZ  = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("LeftMinHi-z"));
    const auto& RightMinHiZ = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("RightMinHi-z"));
    const auto& LeftMaxHiZ  = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("LeftMaxHi-z"));
    const auto& RightMaxHiZ = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("RightMaxHi-z"));

    glm::vec2 noiseScale = static_cast<glm::vec2>(Devices::Get()->GetWindow()->GetSize()) / static_cast<glm::vec2>((*blueNoise)->GetSize());

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

    uniformSSRData.Push("maxRayLength", ssrData.maxRayLength);
    uniformSSRData.Push("maxSteps", ssrData.maxSteps);
    uniformSSRData.Push("zThickness", ssrData.zThickness);
    uniformSSRData.Push("brdfBias", ssrData.brdfBias);
    uniformSSRData.Push("hiZStartLevel", std::min(static_cast<float>(LeftMinHiZ->GetMipLevels()), 2.0f));
    uniformSSRData.Push("hiZMaxLevel", static_cast<float>(LeftMinHiZ->GetMipLevels()) - 1.0f);
    uniformSSRData.Push("noiseScale", noiseScale);

    uniformJitterData.Push("jitter", haltonSampler->next());

    descriptorSet.Push("UniformSSRData", uniformSSRData);
    descriptorSet.Push("UniformJitterData", uniformJitterData);
    descriptorSet.Push("UniformCamera", uniformCamera);
    descriptorSet.Push("inDepth", Graphics::Get()->GetAttachment("depth"));
    descriptorSet.Push("inNormal", Graphics::Get()->GetAttachment("normal"));
    descriptorSet.Push("inMaterial", Graphics::Get()->GetAttachment("material"));
    descriptorSet.Push("inInstanceID", Graphics::Get()->GetAttachment("instanceId"));
    descriptorSet.Push("inPosition", Graphics::Get()->GetAttachment("position"));

    descriptorSet.Push("inMinHiZLeft", LeftMinHiZ);
    descriptorSet.Push("inMinHiZRight", RightMinHiZ);
    descriptorSet.Push("inMaxHiZLeft", LeftMaxHiZ);
    descriptorSet.Push("inMaxHiZRight", RightMaxHiZ);

    descriptorSet.Push("blueNoise", *blueNoise);
    descriptorSet.Push("SSRHitsMap", SSRHitsMap);
    descriptorSet.Push("SSRMask", SSRMask);
    descriptorSet.Push("GlossyMV", glossyMV);
    // descriptorSet.Push("DebugMask", debugMask);

    if (!descriptorSet.Update(pipelineCompute)) return;

    pipelineCompute.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipelineCompute);

    pipelineCompute.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

    SSRHitsMap->Image2dPipelineBarrierComputeToCompute(commandBuffer);
}

std::shared_ptr<Image2d> StochasticSSRStereoSubrender::LoadBlueNoise()
{
    auto blueNoiseImage = Image2d::Create("NoiseImage/BlueNoise.tga", VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, false, false);

    return blueNoiseImage;
}
}   // namespace MapleLeaf