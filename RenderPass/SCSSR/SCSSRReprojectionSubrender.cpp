#include "SCSSRReprojectionSubrender.hpp"

#include "Graphics.hpp"
#include "Image2d.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
SCSSRReprojectionSubrender::SCSSRReprojectionSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineCompute("Shader/SC_SSR/StereoReprojection.comp", {}, false)
{}

void SCSSRReprojectionSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    const auto& SSRHitsMap                     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("SSRHitsMap"));
    const auto& glossyMV                       = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("GlossyMV"));
    const auto& SSRMask                        = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("SSRMask"));
    const auto& ReprojectionReflectionColorMap = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("ReprojectionReflectionColorMap"));
    const auto& BRDFWeightMap                  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BRDFWeightMap"));
    const auto& ReuseMap                       = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("ReuseMap"));

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

    descriptorSet.Push("UniformCamera", uniformCamera);
    descriptorSet.Push("inDepth", Graphics::Get()->GetAttachment("depth"));
    descriptorSet.Push("inNormal", Graphics::Get()->GetAttachment("normal"));
    descriptorSet.Push("inDiffuse", Graphics::Get()->GetAttachment("diffuse"));
    descriptorSet.Push("inMaterial", Graphics::Get()->GetAttachment("material"));
    descriptorSet.Push("LightingMap", Graphics::Get()->GetAttachment("lighting"));
    descriptorSet.Push("SSRHitsMap", SSRHitsMap);
    descriptorSet.Push("SSRHitsMask", SSRMask);
    descriptorSet.Push("GlossyMV", glossyMV);
    descriptorSet.Push("ReprojectionReflectionColorMap", ReprojectionReflectionColorMap);
    descriptorSet.Push("BRDFWeightMap", BRDFWeightMap);
    descriptorSet.Push("ReuseMap", ReuseMap);

    if (!descriptorSet.Update(pipelineCompute)) return;

    pipelineCompute.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipelineCompute);

    pipelineCompute.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

    ReprojectionReflectionColorMap->Image2dPipelineBarrierComputeToCompute(commandBuffer);
    BRDFWeightMap->Image2dPipelineBarrierComputeToCompute(commandBuffer);
}

void SCSSRReprojectionSubrender::Render(const CommandBuffer& commandBuffer) {}

void SCSSRReprojectionSubrender::PostRender(const CommandBuffer& commandBuffer) {}
}   // namespace MapleLeaf