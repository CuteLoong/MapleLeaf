#include "InterpolationBackWarpSubrender.hpp"

#include "DescriptorHandler.hpp"
#include "Graphics.hpp"
#include "Image2d.hpp"
#include "Pipeline.hpp"
#include "PipelineCompute.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {

InterpolationBackWarpSubrender::InterpolationBackWarpSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineWarpDepth("Shader/InterpolationStereo/InterpolationDepth.comp", {}, false)
    , pipelineWarpMV("Shader/InterpolationStereo/InterpolationMV.comp", {}, false)
    , pipelineBlend("Shader/InterpolationStereo/BackwardWarpAlpha.comp", {}, false)
    , pipelineFinement("Shader/InterpolationStereo/RefinementMV.comp", {}, false)
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

    const auto& Zero2OneColor = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2OneColor"));
    const auto& One2ZeroColor = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("One2ZeroColor"));
    const auto& BlockInit     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BlockInit"));


    pushHandlerWarpDepth.Push("alpha", static_cast<float>(0.0));
    pushHandlerWarpMV.Push("alpha", static_cast<float>(0.0));

    Zero2OneDepth_Int->ClearImage2d(commandBuffer, glm::vec4(1.0f));
    One2ZeroDepth_Int->ClearImage2d(commandBuffer, glm::vec4(1.0f));
    Zero2AlphaMV->ClearImage2d(commandBuffer, glm::vec4(0.0f));
    Alpha2OneMV->ClearImage2d(commandBuffer, glm::vec4(0.0f));

    Zero2OneColor->ClearImage2d(commandBuffer, glm::vec4(0.0f));
    One2ZeroColor->ClearImage2d(commandBuffer, glm::vec4(0.0f));

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
    descriptorSetWarpDepth.Push("BlockInit", BlockInit);

    if (!descriptorSetWarpMV.Update(pipelineWarpMV)) return;

    pipelineWarpMV.BindPipeline(commandBuffer);

    descriptorSetWarpMV.BindDescriptor(commandBuffer, pipelineWarpMV);
    pushHandlerWarpMV.BindPush(commandBuffer, pipelineWarpMV);

    pipelineWarpMV.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

    Zero2AlphaMV->Image2dPipelineBarrierComputeToCompute(commandBuffer);
    Alpha2OneMV->Image2dPipelineBarrierComputeToCompute(commandBuffer);

    const auto& FinedZero2AlphaMV = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("FinedZero2AlphaMV"));
    const auto& FinedAlpha2OneMV  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("FinedAlpha2OneMV"));

    descriptorSetFinement.Push("UniformCamera", uniformCameraWarpDepth);
    descriptorSetFinement.Push("Zero2AlphaMV", Zero2AlphaMV);
    descriptorSetFinement.Push("Alpha2OneMV", Alpha2OneMV);
    descriptorSetFinement.Push("FinedZero2AlphaMV", FinedZero2AlphaMV);
    descriptorSetFinement.Push("FinedAlpha2OneMV", FinedAlpha2OneMV);

    if (!descriptorSetFinement.Update(pipelineFinement)) return;

    pipelineFinement.BindPipeline(commandBuffer);

    descriptorSetFinement.BindDescriptor(commandBuffer, pipelineFinement);

    pipelineFinement.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());


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
    const auto& depth             = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("depth"));
    const auto& prevDepth         = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("prevDepth"));
    const auto& lighting          = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("lighting"));
    const auto& prevLighting      = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("PrevLighting"));
    const auto& FinedZero2AlphaMV = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("FinedZero2AlphaMV"));
    const auto& FinedAlpha2OneMV  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("FinedAlpha2OneMV"));

    const auto& AlphaColor    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaColor"));
    const auto& AlphaDepth    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaDepth"));
    const auto& Zero2OneColor = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2OneColor"));
    const auto& One2ZeroColor = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("One2ZeroColor"));

    pushHandlerBlend.Push("alpha", static_cast<float>(0.0));

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCameraBlend);

    descriptorSetBlend.Push("UniformCamera", uniformCameraBlend);
    descriptorSetBlend.Push("lighting", lighting);
    descriptorSetBlend.Push("prevLighting", prevLighting);
    descriptorSetBlend.Push("depth", depth);
    descriptorSetBlend.Push("prevDepth", prevDepth);
    descriptorSetBlend.Push("Zero2AlphaMV", FinedZero2AlphaMV);
    descriptorSetBlend.Push("Alpha2OneMV", FinedAlpha2OneMV);
    descriptorSetBlend.Push("PushObject", pushHandlerBlend);

    descriptorSetBlend.Push("AlphaColor", AlphaColor);
    descriptorSetBlend.Push("AlphaDepth", AlphaDepth);
    descriptorSetBlend.Push("Zero2OneColor", Zero2OneColor);
    descriptorSetBlend.Push("One2ZeroColor", One2ZeroColor);


    if (!descriptorSetBlend.Update(pipelineBlend)) return;

    pipelineBlend.BindPipeline(commandBuffer);

    descriptorSetBlend.BindDescriptor(commandBuffer, pipelineBlend);
    pushHandlerBlend.BindPush(commandBuffer, pipelineBlend);

    pipelineBlend.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());
}

void InterpolationBackWarpSubrender::ComputeBlend()
{
    PipelineCompute computeWarpDepth("Shader/InterpolationStereo/InterpolationDepth.comp", {}, false);
    PipelineCompute computeWarpMV("Shader/InterpolationStereo/InterpolationMV.comp", {}, false);
    PipelineCompute computeBlend("Shader/InterpolationStereo/BackwardWarpAlpha.comp", {}, false);
    PipelineCompute computeFinement("Shader/InterpolationStereo/RefinementMV.comp", {}, false);

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
    const auto& Zero2OneColor     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2OneColor"));
    const auto& One2ZeroColor     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("One2ZeroColor"));
    const auto& AlphaColor        = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaColor"));
    const auto& AlphaDepth        = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaDepth"));
    const auto& FinedZero2AlphaMV = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("FinedZero2AlphaMV"));
    const auto& FinedAlpha2OneMV  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("FinedAlpha2OneMV"));

    DescriptorsHandler descriptorSet_1(computeWarpDepth);
    DescriptorsHandler descriptorSet_2(computeWarpMV);
    DescriptorsHandler descriptorSet_3(computeFinement);
    DescriptorsHandler descriptorSet_4(computeBlend);

    for (int i = 0; i <= 30; i++) {
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

        descriptorSet_3.Push("UniformCamera", uniformCameraWarpDepth);
        descriptorSet_3.Push("Zero2AlphaMV", Zero2AlphaMV);
        descriptorSet_3.Push("Alpha2OneMV", Alpha2OneMV);
        descriptorSet_3.Push("FinedZero2AlphaMV", FinedZero2AlphaMV);
        descriptorSet_3.Push("FinedAlpha2OneMV", FinedAlpha2OneMV);

        descriptorSet_3.Update(computeFinement);

        computeFinement.BindPipeline(commandBuffer_3);
        descriptorSet_3.BindDescriptor(commandBuffer_3, computeFinement);

        computeFinement.CmdRender(commandBuffer_3, Graphics::Get()->GetNonRTAttachmentSize());

        FinedZero2AlphaMV->Image2dPipelineBarrierComputeToCompute(commandBuffer_3);
        FinedAlpha2OneMV->Image2dPipelineBarrierComputeToCompute(commandBuffer_3);

        commandBuffer_3.SubmitIdle();

        CommandBuffer commandBuffer_4(true, VK_QUEUE_COMPUTE_BIT);

        descriptorSet_4.Push("UniformCamera", uniformCameraBlend);
        descriptorSet_4.Push("lighting", lighting);
        descriptorSet_4.Push("prevLighting", prevLighting);
        descriptorSet_4.Push("depth", depth);
        descriptorSet_4.Push("prevDepth", prevDepth);
        descriptorSet_4.Push("Zero2AlphaMV", FinedZero2AlphaMV);
        descriptorSet_4.Push("Alpha2OneMV", FinedAlpha2OneMV);
        descriptorSet_4.Push("PushObject", pushHandlerWarpDepth);

        descriptorSet_4.Push("AlphaColor", AlphaColor);
        descriptorSet_4.Push("AlphaDepth", AlphaDepth);
        descriptorSet_4.Push("Zero2OneColor", Zero2OneColor);
        descriptorSet_4.Push("One2ZeroColor", One2ZeroColor);

        descriptorSet_4.Update(computeBlend);

        computeBlend.BindPipeline(commandBuffer_4);
        descriptorSet_4.BindDescriptor(commandBuffer_4, computeBlend);
        pushHandlerWarpDepth.BindPush(commandBuffer_4, computeBlend);

        computeBlend.CmdRender(commandBuffer_4, Graphics::Get()->GetNonRTAttachmentSize());

        AlphaColor->Image2dPipelineBarrierComputeToCompute(commandBuffer_4);
        AlphaDepth->Image2dPipelineBarrierComputeToCompute(commandBuffer_4);

        commandBuffer_4.SubmitIdle();

        Graphics::Get()->CaptureImage2d("Screenshots/AlphaColor" + std::to_string(i) + ".png", AlphaColor);
        Graphics::Get()->CaptureImage2d("Screenshots/AlphaDepth" + std::to_string(i) + ".png", AlphaDepth);
    }
}

}   // namespace MapleLeaf