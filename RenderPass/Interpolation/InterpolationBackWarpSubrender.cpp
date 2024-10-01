#include "InterpolationBackWarpSubrender.hpp"

#include "Graphics.hpp"
#include "Image2d.hpp"
#include "PipelineCompute.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {

InterpolationBackWarpSubrender::InterpolationBackWarpSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineWarpDepth("Shader/Interpolation/InterpolationDepth.comp", {}, false)
    , pipelineWarpMV("Shader/Interpolation/InterpolationMV.comp", {}, false)
    , pipelineBlend("Shader/Interpolation/BackwardWarpAlpha.comp", {}, false)
    , uniformCameraWarpDepth(true)
    , pushHandlerWarpMV(true)
    , uniformCameraBlend(true)
    , pushHandlerWarpDepth(true)
{}

void InterpolationBackWarpSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    const auto& prevDepth        = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("prevDepth"));
    const auto& depth            = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("depth"));
    const auto& prevMotionVector = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("prevMotionVector"));
    const auto& motionVector     = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("motionVector"));

    const auto& Zero2OneDepth_Int = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2OneDepth_Int"));
    const auto& One2ZeroDepth_Int = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("One2ZeroDepth_Int"));
    const auto& Zero2AlphaMV      = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2AlphaMV"));
    const auto& Alpha2OneMV       = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Alpha2OneMV"));

    pushHandlerWarpDepth.Push("alpha", static_cast<float>(0.7));
    pushHandlerWarpMV.Push("alpha", static_cast<float>(0.7));

    Zero2OneDepth_Int->ClearImage2d(commandBuffer, glm::vec4(1.0f));
    One2ZeroDepth_Int->ClearImage2d(commandBuffer, glm::vec4(1.0f));
    Zero2AlphaMV->ClearImage2d(commandBuffer, glm::vec4(0.0f));
    Alpha2OneMV->ClearImage2d(commandBuffer, glm::vec4(0.0f));

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCameraWarpDepth);
    camera->PushUniforms(uniformCameraWarpMV);

    descriptorSetWarpDepth.Push("UniformCamera", uniformCameraWarpDepth);
    descriptorSetWarpDepth.Push("prevDepth", prevDepth);
    descriptorSetWarpDepth.Push("depth", depth);
    descriptorSetWarpDepth.Push("prevMotionVector", prevMotionVector);
    descriptorSetWarpDepth.Push("motionVector", motionVector);
    descriptorSetWarpDepth.Push("PushObject", pushHandlerWarpDepth);

    descriptorSetWarpDepth.Push("Zero2OneDepth_Int", Zero2OneDepth_Int);
    descriptorSetWarpDepth.Push("One2ZeroDepth_Int", One2ZeroDepth_Int);

    if (!descriptorSetWarpDepth.Update(pipelineWarpDepth)) return;

    pipelineWarpDepth.BindPipeline(commandBuffer);

    descriptorSetWarpDepth.BindDescriptor(commandBuffer, pipelineWarpDepth);
    pushHandlerWarpDepth.BindPush(commandBuffer, pipelineWarpDepth);

    pipelineWarpDepth.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

    Zero2OneDepth_Int->Image2dPipelineBarrierComputeToCompute(commandBuffer);
    One2ZeroDepth_Int->Image2dPipelineBarrierComputeToCompute(commandBuffer);

    descriptorSetWarpMV.Push("UniformCamera", uniformCameraWarpMV);
    descriptorSetWarpMV.Push("prevDepth", prevDepth);
    descriptorSetWarpMV.Push("depth", depth);
    descriptorSetWarpMV.Push("prevMotionVector", prevMotionVector);
    descriptorSetWarpMV.Push("motionVector", motionVector);
    descriptorSetWarpMV.Push("Zero2OneDepth_Int", Zero2OneDepth_Int);
    descriptorSetWarpMV.Push("One2ZeroDepth_Int", One2ZeroDepth_Int);
    descriptorSetWarpMV.Push("PushObject", pushHandlerWarpMV);

    descriptorSetWarpMV.Push("Zero2AlphaMV", Zero2AlphaMV);
    descriptorSetWarpMV.Push("Alpha2OneMV", Alpha2OneMV);

    if (!descriptorSetWarpMV.Update(pipelineWarpMV)) return;

    pipelineWarpMV.BindPipeline(commandBuffer);

    descriptorSetWarpMV.BindDescriptor(commandBuffer, pipelineWarpMV);
    pushHandlerWarpMV.BindPush(commandBuffer, pipelineWarpMV);

    pipelineWarpMV.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

    if (frameID == 8) {
        ComputeBlend();
        exit(0);
    }
}

void InterpolationBackWarpSubrender::Render(const CommandBuffer& commandBuffer)
{
    frameID++;
}

void InterpolationBackWarpSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    const auto& depth        = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("depth"));
    const auto& prevDepth    = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("prevDepth"));
    const auto& lighting     = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("lighting"));
    const auto& prevLighting = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("PrevLighting"));
    const auto& Zero2AlphaMV = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2AlphaMV"));
    const auto& Alpha2OneMV  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Alpha2OneMV"));

    const auto& AlphaColor = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaColor"));
    const auto& AlphaDepth = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaDepth"));

    pushHandlerBlend.Push("alpha", static_cast<float>(0.7));

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCameraBlend);

    descriptorSetBlend.Push("UniformCamera", uniformCameraBlend);
    descriptorSetBlend.Push("lighting", lighting);
    descriptorSetBlend.Push("prevLighting", prevLighting);
    descriptorSetBlend.Push("depth", depth);
    descriptorSetBlend.Push("prevDepth", prevDepth);
    descriptorSetBlend.Push("Zero2AlphaMV", Zero2AlphaMV);
    descriptorSetBlend.Push("Alpha2OneMV", Alpha2OneMV);
    descriptorSetBlend.Push("PushObject", pushHandlerBlend);

    descriptorSetBlend.Push("AlphaColor", AlphaColor);
    descriptorSetBlend.Push("AlphaDepth", AlphaDepth);

    if (!descriptorSetBlend.Update(pipelineBlend)) return;

    pipelineBlend.BindPipeline(commandBuffer);

    descriptorSetBlend.BindDescriptor(commandBuffer, pipelineBlend);
    pushHandlerBlend.BindPush(commandBuffer, pipelineBlend);

    pipelineBlend.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());
}

void InterpolationBackWarpSubrender::ComputeBlend()
{
    PipelineCompute computeWarpDepth("Shader/Interpolation/InterpolationDepth.comp", {}, false);
    PipelineCompute computeWarpMV("Shader/Interpolation/InterpolationMV.comp", {}, false);
    PipelineCompute computeBlend("Shader/Interpolation/BackwardWarpAlpha.comp", {}, false);

    const auto& prevLighting     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("PrevLighting"));
    const auto& lighting         = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("lighting"));
    const auto& prevDepth        = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("prevDepth"));
    const auto& depth            = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("depth"));
    const auto& prevMotionVector = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("prevMotionVector"));
    const auto& motionVector     = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("motionVector"));

    const auto& Zero2OneDepth_Int = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2OneDepth_Int"));
    const auto& One2ZeroDepth_Int = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("One2ZeroDepth_Int"));
    const auto& Zero2AlphaMV      = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2AlphaMV"));
    const auto& Alpha2OneMV       = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Alpha2OneMV"));
    const auto& AlphaColor        = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaColor"));
    const auto& AlphaDepth        = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaDepth"));

    DescriptorsHandler descriptorSet_1(computeWarpDepth);
    DescriptorsHandler descriptorSet_2(computeWarpMV);
    DescriptorsHandler descriptorSet_3(computeBlend);

    for (int i = 0; i < 30; i++) {
        pushHandlerWarpDepth.Push("alpha", static_cast<float>(i) / static_cast<float>(30));

        CommandBuffer commandBuffer_1(true, VK_QUEUE_COMPUTE_BIT);

        Zero2OneDepth_Int->ClearImage2d(commandBuffer_1, glm::vec4(1.0f));
        One2ZeroDepth_Int->ClearImage2d(commandBuffer_1, glm::vec4(1.0f));
        Zero2AlphaMV->ClearImage2d(commandBuffer_1, glm::vec4(0.0f));
        Alpha2OneMV->ClearImage2d(commandBuffer_1, glm::vec4(0.0f));

        descriptorSet_1.Push("UniformCamera", uniformCameraWarpDepth);
        descriptorSet_1.Push("prevDepth", prevDepth);
        descriptorSet_1.Push("depth", depth);
        descriptorSet_1.Push("prevMotionVector", prevMotionVector);
        descriptorSet_1.Push("motionVector", motionVector);
        descriptorSet_1.Push("PushObject", pushHandlerWarpDepth);

        descriptorSet_1.Push("Zero2OneDepth_Int", Zero2OneDepth_Int);
        descriptorSet_1.Push("One2ZeroDepth_Int", One2ZeroDepth_Int);

        descriptorSet_1.Update(computeWarpDepth);
        computeWarpDepth.BindPipeline(commandBuffer_1);
        descriptorSet_1.BindDescriptor(commandBuffer_1, computeWarpDepth);
        pushHandlerWarpDepth.BindPush(commandBuffer_1, computeWarpDepth);

        computeWarpDepth.CmdRender(commandBuffer_1, Graphics::Get()->GetNonRTAttachmentSize());

        Zero2OneDepth_Int->Image2dPipelineBarrierComputeToCompute(commandBuffer_1);
        One2ZeroDepth_Int->Image2dPipelineBarrierComputeToCompute(commandBuffer_1);

        commandBuffer_1.SubmitIdle();

        CommandBuffer commandBuffer_2(true, VK_QUEUE_COMPUTE_BIT);

        descriptorSet_2.Push("UniformCamera", uniformCameraWarpMV);
        descriptorSet_2.Push("prevDepth", prevDepth);
        descriptorSet_2.Push("depth", depth);
        descriptorSet_2.Push("prevMotionVector", prevMotionVector);
        descriptorSet_2.Push("motionVector", motionVector);
        descriptorSet_2.Push("Zero2OneDepth_Int", Zero2OneDepth_Int);
        descriptorSet_2.Push("One2ZeroDepth_Int", One2ZeroDepth_Int);
        descriptorSet_2.Push("PushObject", pushHandlerWarpDepth);

        descriptorSet_2.Push("Zero2AlphaMV", Zero2AlphaMV);
        descriptorSet_2.Push("Alpha2OneMV", Alpha2OneMV);

        descriptorSet_2.Update(computeWarpMV);
        computeWarpMV.BindPipeline(commandBuffer_2);
        descriptorSet_2.BindDescriptor(commandBuffer_2, computeWarpMV);
        pushHandlerWarpDepth.BindPush(commandBuffer_2, computeWarpMV);

        computeWarpMV.CmdRender(commandBuffer_2, Graphics::Get()->GetNonRTAttachmentSize());

        Zero2AlphaMV->Image2dPipelineBarrierComputeToCompute(commandBuffer_2);
        Alpha2OneMV->Image2dPipelineBarrierComputeToCompute(commandBuffer_2);

        commandBuffer_2.SubmitIdle();

        CommandBuffer commandBuffer_3(true, VK_QUEUE_COMPUTE_BIT);

        descriptorSet_3.Push("UniformCamera", uniformCameraBlend);
        descriptorSet_3.Push("lighting", lighting);
        descriptorSet_3.Push("prevLighting", prevLighting);
        descriptorSet_3.Push("depth", depth);
        descriptorSet_3.Push("prevDepth", prevDepth);
        descriptorSet_3.Push("Zero2AlphaMV", Zero2AlphaMV);
        descriptorSet_3.Push("Alpha2OneMV", Alpha2OneMV);
        descriptorSet_3.Push("PushObject", pushHandlerWarpDepth);

        descriptorSet_3.Push("AlphaColor", AlphaColor);
        descriptorSet_3.Push("AlphaDepth", AlphaDepth);

        descriptorSet_3.Update(computeBlend);

        computeBlend.BindPipeline(commandBuffer_3);
        descriptorSet_3.BindDescriptor(commandBuffer_3, computeBlend);
        pushHandlerWarpDepth.BindPush(commandBuffer_3, computeBlend);

        computeBlend.CmdRender(commandBuffer_3, Graphics::Get()->GetNonRTAttachmentSize());

        AlphaColor->Image2dPipelineBarrierComputeToCompute(commandBuffer_3);
        AlphaDepth->Image2dPipelineBarrierComputeToCompute(commandBuffer_3);

        commandBuffer_3.SubmitIdle();

        Graphics::Get()->CaptureImage2d("Screenshots/AlphaColor" + std::to_string(i) + ".png", AlphaColor);
        Graphics::Get()->CaptureImage2d("Screenshots/AlphaDepth" + std::to_string(i) + ".png", AlphaDepth);
    }
}

}   // namespace MapleLeaf