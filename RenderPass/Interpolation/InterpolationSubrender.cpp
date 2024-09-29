#include "InterpolationSubrender.hpp"

#include "Graphics.hpp"
#include "Image2d.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {

InterpolationSubrender::InterpolationSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineWarpDepth("Shader/Interpolation/InterpolationDepth.comp", {}, false)
    , pipelineWarpColor("Shader/Interpolation/InterpolationColor.comp", {}, false)
    , pipelineBlend("Shader/Interpolation/Blend.comp", {}, false)
{}

void InterpolationSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    const auto& prevLighting     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("PrevLighting"));
    const auto& lighting         = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("lighting"));
    const auto& prevDepth        = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("prevDepth"));
    const auto& depth            = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("depth"));
    const auto& prevMotionVector = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("prevMotionVector"));
    const auto& motionVector     = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("motionVector"));

    const auto& Zero2OneColor     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2OneColor"));
    const auto& One2ZeroColor     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("One2ZeroColor"));
    const auto& Zero2OneDepth_Int = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2OneDepth_Int"));
    const auto& One2ZeroDepth_Int = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("One2ZeroDepth_Int"));
    const auto& Zero2OneDepth     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2OneDepth"));
    const auto& One2ZeroDepth     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("One2ZeroDepth"));

    Zero2OneDepth_Int->ClearImage2d(commandBuffer, glm::vec4(1.0f));
    One2ZeroDepth_Int->ClearImage2d(commandBuffer, glm::vec4(1.0f));
    Zero2OneDepth->ClearImage2d(commandBuffer, glm::vec4(0.0f));
    One2ZeroDepth->ClearImage2d(commandBuffer, glm::vec4(0.0f));
    Zero2OneColor->ClearImage2d(commandBuffer, glm::vec4(0.0f));
    One2ZeroColor->ClearImage2d(commandBuffer, glm::vec4(0.0f));

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCameraWarpDepth);
    camera->PushUniforms(uniformCameraWarpColor);

    descriptorSetWarpDepth.Push("UniformCamera", uniformCameraWarpDepth);
    descriptorSetWarpDepth.Push("prevDepth", prevDepth);
    descriptorSetWarpDepth.Push("depth", depth);
    descriptorSetWarpDepth.Push("prevMotionVector", prevMotionVector);
    descriptorSetWarpDepth.Push("motionVector", motionVector);

    descriptorSetWarpDepth.Push("Zero2OneDepth_Int", Zero2OneDepth_Int);
    descriptorSetWarpDepth.Push("One2ZeroDepth_Int", One2ZeroDepth_Int);

    if (!descriptorSetWarpDepth.Update(pipelineWarpDepth)) return;

    pipelineWarpDepth.BindPipeline(commandBuffer);

    descriptorSetWarpDepth.BindDescriptor(commandBuffer, pipelineWarpDepth);

    pipelineWarpDepth.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

    Zero2OneDepth_Int->Image2dPipelineBarrierComputeToCompute(commandBuffer);
    One2ZeroDepth_Int->Image2dPipelineBarrierComputeToCompute(commandBuffer);

    descriptorSetWarpColor.Push("UniformCamera", uniformCameraWarpColor);
    descriptorSetWarpColor.Push("prevLighting", prevLighting);
    descriptorSetWarpColor.Push("lighting", lighting);
    descriptorSetWarpColor.Push("prevDepth", prevDepth);
    descriptorSetWarpColor.Push("depth", depth);
    descriptorSetWarpColor.Push("prevMotionVector", prevMotionVector);
    descriptorSetWarpColor.Push("motionVector", motionVector);
    descriptorSetWarpColor.Push("Zero2OneDepth_Int", Zero2OneDepth_Int);
    descriptorSetWarpColor.Push("One2ZeroDepth_Int", One2ZeroDepth_Int);

    descriptorSetWarpColor.Push("Zero2OneColor", Zero2OneColor);
    descriptorSetWarpColor.Push("One2ZeroColor", One2ZeroColor);
    descriptorSetWarpColor.Push("Zero2OneDepth", Zero2OneDepth);
    descriptorSetWarpColor.Push("One2ZeroDepth", One2ZeroDepth);

    if (!descriptorSetWarpColor.Update(pipelineWarpColor)) return;

    pipelineWarpColor.BindPipeline(commandBuffer);

    descriptorSetWarpColor.BindDescriptor(commandBuffer, pipelineWarpColor);

    pipelineWarpColor.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());
}

void InterpolationSubrender::Render(const CommandBuffer& commandBuffer) {}

void InterpolationSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    const auto& Zero2OneColor = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2OneColor"));
    const auto& Zero2OneDepth = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2OneDepth"));
    const auto& One2ZeroColor = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("One2ZeroColor"));
    const auto& One2ZeroDepth = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("One2ZeroDepth"));
    const auto& AlphaColor    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaColor"));
    const auto& AlphaDepth    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaDepth"));

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCameraBlend);

    descriptorSetBlend.Push("UniformCamera", uniformCameraBlend);
    descriptorSetBlend.Push("Zero2OneColor", Zero2OneColor);
    descriptorSetBlend.Push("Zero2OneDepth", Zero2OneDepth);
    descriptorSetBlend.Push("One2ZeroColor", One2ZeroColor);
    descriptorSetBlend.Push("One2ZeroDepth", One2ZeroDepth);
    descriptorSetBlend.Push("AlphaColor", AlphaColor);
    descriptorSetBlend.Push("AlphaDepth", AlphaDepth);

    if (!descriptorSetBlend.Update(pipelineBlend)) return;

    pipelineBlend.BindPipeline(commandBuffer);

    descriptorSetBlend.BindDescriptor(commandBuffer, pipelineBlend);

    pipelineBlend.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());
}

}   // namespace MapleLeaf