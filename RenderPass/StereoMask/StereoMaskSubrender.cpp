#include "StereoMaskSubrender.hpp"

#include "Graphics.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
StereoMaskSubrender::StereoMaskSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , uniformCamera(true)
    , pipelineGradient("Shader/StereoMask/StereoThickness.comp")
    , pipeline(PipelineGraphics(pipelineStage, {"Shader/StereoMask/StereoMask.vert", "Shader/StereoMask/StereoMask.frag"}, {}, {},
                                PipelineGraphics::Mode::MRT, PipelineGraphics::Depth::None))
{}

void StereoMaskSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void StereoMaskSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCamera);

    descriptorSet.Push("UniformCamera", uniformCamera);
    descriptorSet.Push("inDepth", Graphics::Get()->GetAttachment("depth"));
    descriptorSet.Push("inPosition", Graphics::Get()->GetAttachment("position"));

    if (!descriptorSet.Update(pipeline)) return;

    // Draws the quad.
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void StereoMaskSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    const auto& stereoMask   = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("StereoMask"));
    const auto& stereoMV     = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("StereoMV"));
    const auto& thicknessMap = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("ThicknessMap"));

    stereoMask->Image2dPipelineBarrierGraphicToCompute(commandBuffer);
    stereoMV->Image2dPipelineBarrierGraphicToCompute(commandBuffer);

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCameraCompute);

    descriptorSetCompute.Push("UniformCamera", uniformCameraCompute);
    descriptorSetCompute.Push("inDepth", Graphics::Get()->GetAttachment("depth"));
    descriptorSetCompute.Push("inInstanceID", Graphics::Get()->GetAttachment("instanceId"));
    descriptorSetCompute.Push("inMask", stereoMask);
    descriptorSetCompute.Push("inMV", stereoMV);
    descriptorSetCompute.Push("thicknessMap", thicknessMap);

    if (!descriptorSetCompute.Update(pipelineGradient)) return;

    pipelineGradient.BindPipeline(commandBuffer);
    descriptorSetCompute.BindDescriptor(commandBuffer, pipelineGradient);
    pipelineGradient.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

    thicknessMap->Image2dPipelineBarrierComputeToGraphic(commandBuffer);
}

// calc parallax gradient, not useful
// const auto& stereoMask       = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("StereoMask"));
// const auto& parallaxGradient = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("ParallaxGradient"));

// stereoMask->Image2dPipelineBarrierGraphicToCompute(commandBuffer);

// descriptorSetCompute.Push("UniformCamera", uniformCamera);
// descriptorSetCompute.Push("inputImage", stereoMask);
// descriptorSetCompute.Push("outputImage", parallaxGradient);

// if (!descriptorSetCompute.Update(pipelineGradient)) return;

// pipelineGradient.BindPipeline(commandBuffer);
// descriptorSetCompute.BindDescriptor(commandBuffer, pipelineGradient);
// pipelineGradient.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

// parallaxGradient->Image2dPipelineBarrierComputeToGraphic(commandBuffer);
}   // namespace MapleLeaf
