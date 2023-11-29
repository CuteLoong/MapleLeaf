#include "SSRStereoSubrender.hpp"

#include "Graphics.hpp"
#include "Image2d.hpp"
#include "Scenes.hpp"


namespace MapleLeaf {
SSRStereoSubrender::SSRStereoSubrender(const Pipeline::Stage& pipelineStage, SSRData ssrData)
    : Subrender(pipelineStage)
    , ssrData(ssrData)
    , pipelineCompute("Shader/SSR/SSRStereoComplement.comp", {SSRStereoSubrender::GetDefines()}, false)
    , brdf(Resources::Get()->GetThreadPool().Enqueue(ComputeBRDF, 512))
{}

void SSRStereoSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void SSRStereoSubrender::Render(const CommandBuffer& commandBuffer) {}

void SSRStereoSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    const auto& SSRHitsMap = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("SSRHitsMap"));

    const auto& minHiZ = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("MinHi-z"));
    const auto& maxHiZ = dynamic_cast<const ImageHierarchyZ*>(Graphics::Get()->GetNonRTAttachment("MaxHi-z"));

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

    uniformSSRData.Push("maxRayLength", ssrData.maxRayLength);
    uniformSSRData.Push("maxSteps", ssrData.maxSteps);
    uniformSSRData.Push("zThickness", ssrData.zThickness);
    uniformSSRData.Push("fadeValue", ssrData.fadeValue);
    uniformSSRData.Push("hiZStartLevel", std::min(static_cast<float>(minHiZ->GetMipLevels()), 2.0f));
    uniformSSRData.Push("hiZMaxLevel", static_cast<float>(minHiZ->GetMipLevels()) - 1.0f);

    uniformSSRData.Push("fadeValue", ssrData.fadeValue);


    descriptorSet.Push("UniformSSRData", uniformSSRData);
    descriptorSet.Push("UniformCamera", uniformCamera);
    descriptorSet.Push("inDepth", Graphics::Get()->GetAttachment("depth"));
    descriptorSet.Push("inNormal", Graphics::Get()->GetAttachment("normal"));
    descriptorSet.Push("inMinHiZ", Graphics::Get()->GetNonRTAttachment("MinHi-z"));
    descriptorSet.Push("inMaxHiZ", Graphics::Get()->GetNonRTAttachment("MaxHi-z"));
    descriptorSet.Push("SSRHitsMap", SSRHitsMap);
    descriptorSet.Push("preIntegratedDFG", *brdf);

    if (!descriptorSet.Update(pipelineCompute)) return;

    pipelineCompute.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipelineCompute);

    pipelineCompute.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

    SSRHitsMap->Image2dPipelineBarrierComputeToGraphic(commandBuffer);
}

std::vector<Shader::Define> SSRStereoSubrender::GetDefines()
{
    return {{"TRAVERSAL_SCHEME_RAY_MARCH_3D", std::to_string(0)},
            {"TRAVERSAL_SCHEME_MIN_HI_Z", std::to_string(0)},
            {"TRAVERSAL_SCHEME_MIN_MAX_HI_Z", std::to_string(1)}};
}

std::unique_ptr<Image2d> SSRStereoSubrender::ComputeBRDF(uint32_t size)
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
}   // namespace MapleLeaf