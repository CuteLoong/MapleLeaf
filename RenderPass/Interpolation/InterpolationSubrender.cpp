#include "InterpolationSubrender.hpp"

#include "Graphics.hpp"
#include "Image2d.hpp"
#include "PipelineCompute.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {

InterpolationSubrender::InterpolationSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineWarpDepth("Shader/Interpolation/InterpolationDepth.comp", {}, false)
    , pipelineWarpColor("Shader/Interpolation/InterpolationColor.comp", {}, false)
    , pipelineBlend("Shader/Interpolation/Blend.comp", {}, false)
    , uniformCameraWarpDepth(true)
    , pushHandlerWarpColor(true)
    , uniformCameraBlend(true)
    , pushHandlerWarpDepth(true)
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

    // if (frameID == 12) {
    //     Graphics::Get()->CaptureImage2d("Screenshots/lighting20_12.png", lighting);
    // }

    pushHandlerWarpDepth.Push("alpha", static_cast<float>(0.0));
    pushHandlerWarpColor.Push("alpha", static_cast<float>(0.0));

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

    descriptorSetWarpColor.Push("UniformCamera", uniformCameraWarpColor);
    descriptorSetWarpColor.Push("prevLighting", prevLighting);
    descriptorSetWarpColor.Push("lighting", lighting);
    descriptorSetWarpColor.Push("prevDepth", prevDepth);
    descriptorSetWarpColor.Push("depth", depth);
    descriptorSetWarpColor.Push("prevMotionVector", prevMotionVector);
    descriptorSetWarpColor.Push("motionVector", motionVector);
    descriptorSetWarpColor.Push("Zero2OneDepth_Int", Zero2OneDepth_Int);
    descriptorSetWarpColor.Push("One2ZeroDepth_Int", One2ZeroDepth_Int);
    descriptorSetWarpColor.Push("PushObject", pushHandlerWarpColor);

    descriptorSetWarpColor.Push("Zero2OneColor", Zero2OneColor);
    descriptorSetWarpColor.Push("One2ZeroColor", One2ZeroColor);
    descriptorSetWarpColor.Push("Zero2OneDepth", Zero2OneDepth);
    descriptorSetWarpColor.Push("One2ZeroDepth", One2ZeroDepth);

    if (!descriptorSetWarpColor.Update(pipelineWarpColor)) return;

    pipelineWarpColor.BindPipeline(commandBuffer);

    descriptorSetWarpColor.BindDescriptor(commandBuffer, pipelineWarpColor);
    pushHandlerWarpColor.BindPush(commandBuffer, pipelineWarpColor);

    pipelineWarpColor.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());
}

void InterpolationSubrender::Render(const CommandBuffer& commandBuffer)
{
    frameID++;
}

void InterpolationSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    const auto& Zero2OneColor = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2OneColor"));
    const auto& Zero2OneDepth = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("Zero2OneDepth"));
    const auto& One2ZeroColor = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("One2ZeroColor"));
    const auto& One2ZeroDepth = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("One2ZeroDepth"));
    const auto& AlphaColor    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaColor"));
    const auto& AlphaDepth    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaDepth"));

    pushHandlerBlend.Push("alpha", static_cast<float>(0.0));

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCameraBlend);

    descriptorSetBlend.Push("UniformCamera", uniformCameraBlend);
    descriptorSetBlend.Push("Zero2OneColor", Zero2OneColor);
    descriptorSetBlend.Push("Zero2OneDepth", Zero2OneDepth);
    descriptorSetBlend.Push("One2ZeroColor", One2ZeroColor);
    descriptorSetBlend.Push("One2ZeroDepth", One2ZeroDepth);
    descriptorSetBlend.Push("PushObject", pushHandlerBlend);

    descriptorSetBlend.Push("AlphaColor", AlphaColor);
    descriptorSetBlend.Push("AlphaDepth", AlphaDepth);

    if (!descriptorSetBlend.Update(pipelineBlend)) return;

    pipelineBlend.BindPipeline(commandBuffer);

    descriptorSetBlend.BindDescriptor(commandBuffer, pipelineBlend);
    pushHandlerBlend.BindPush(commandBuffer, pipelineBlend);

    pipelineBlend.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

    if (frameID == 9) {
        ComputeBlend();
        exit(0);
    }
}

void InterpolationSubrender::ComputeBlend()
{
    PipelineCompute computeDepth("Shader/Interpolation/InterpolationDepth.comp");
    PipelineCompute computeColor("Shader/Interpolation/InterpolationColor.comp");
    PipelineCompute computeBlend("Shader/Interpolation/Blend.comp");

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

    const auto& AlphaColor = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaColor"));
    const auto& AlphaDepth = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaDepth"));

    auto camera = Scenes::Get()->GetScene()->GetCamera();

    DescriptorsHandler descriptorSet_1(computeDepth);
    DescriptorsHandler descriptorSet_2(computeColor);
    DescriptorsHandler descriptorSet_3(computeBlend);

    for (int i = 0; i < 30; i++) {
        // pushHandler_1.Push("alpha", static_cast<float>(i) / static_cast<float>(30));
        // pushHandler_2.Push("alpha", static_cast<float>(i) / static_cast<float>(30));
        // pushHandler_3.Push("alpha", static_cast<float>(i) / static_cast<float>(30));
        pushHandlerWarpDepth.Push("alpha", static_cast<float>(i) / static_cast<float>(30));

        CommandBuffer commandBuffer_1(true, VK_QUEUE_COMPUTE_BIT);

        // commandBuffer_1.Begin();
        Zero2OneDepth_Int->ClearImage2d(commandBuffer_1, glm::vec4(1.0f));
        One2ZeroDepth_Int->ClearImage2d(commandBuffer_1, glm::vec4(1.0f));
        Zero2OneDepth->ClearImage2d(commandBuffer_1, glm::vec4(0.0f));
        One2ZeroDepth->ClearImage2d(commandBuffer_1, glm::vec4(0.0f));
        Zero2OneColor->ClearImage2d(commandBuffer_1, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        One2ZeroColor->ClearImage2d(commandBuffer_1, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        descriptorSet_1.Push("UniformCamera", uniformCameraWarpDepth);
        descriptorSet_1.Push("prevDepth", prevDepth);
        descriptorSet_1.Push("depth", depth);
        descriptorSet_1.Push("prevMotionVector", prevMotionVector);
        descriptorSet_1.Push("motionVector", motionVector);
        descriptorSet_1.Push("PushObject", pushHandlerWarpDepth);

        descriptorSet_1.Push("Zero2OneDepth_Int", Zero2OneDepth_Int);
        descriptorSet_1.Push("One2ZeroDepth_Int", One2ZeroDepth_Int);

        descriptorSet_1.Update(computeDepth);
        computeDepth.BindPipeline(commandBuffer_1);
        descriptorSet_1.BindDescriptor(commandBuffer_1, computeDepth);
        pushHandlerWarpDepth.BindPush(commandBuffer_1, computeDepth);

        computeDepth.CmdRender(commandBuffer_1, Graphics::Get()->GetNonRTAttachmentSize());

        Zero2OneDepth_Int->Image2dPipelineBarrierComputeToCompute(commandBuffer_1);
        One2ZeroDepth_Int->Image2dPipelineBarrierComputeToCompute(commandBuffer_1);

        commandBuffer_1.SubmitIdle();

        CommandBuffer commandBuffer_2(true, VK_QUEUE_COMPUTE_BIT);

        // commandBuffer_2.Begin();
        descriptorSet_2.Push("UniformCamera", uniformCameraWarpColor);
        descriptorSet_2.Push("prevLighting", prevLighting);
        descriptorSet_2.Push("lighting", lighting);
        descriptorSet_2.Push("prevDepth", prevDepth);
        descriptorSet_2.Push("depth", depth);
        descriptorSet_2.Push("prevMotionVector", prevMotionVector);
        descriptorSet_2.Push("motionVector", motionVector);
        descriptorSet_2.Push("Zero2OneDepth_Int", Zero2OneDepth_Int);
        descriptorSet_2.Push("One2ZeroDepth_Int", One2ZeroDepth_Int);
        descriptorSet_2.Push("PushObject", pushHandlerWarpDepth);

        descriptorSet_2.Push("Zero2OneColor", Zero2OneColor);
        descriptorSet_2.Push("One2ZeroColor", One2ZeroColor);
        descriptorSet_2.Push("Zero2OneDepth", Zero2OneDepth);
        descriptorSet_2.Push("One2ZeroDepth", One2ZeroDepth);

        descriptorSet_2.Update(computeColor);
        computeColor.BindPipeline(commandBuffer_2);
        descriptorSet_2.BindDescriptor(commandBuffer_2, computeColor);
        pushHandlerWarpDepth.BindPush(commandBuffer_2, computeColor);

        computeColor.CmdRender(commandBuffer_2, Graphics::Get()->GetNonRTAttachmentSize());

        commandBuffer_2.SubmitIdle();

        Graphics::Get()->CaptureImage2d("Screenshots/Zero2OneColor" + std::to_string(i) + ".png", Zero2OneColor);
        Graphics::Get()->CaptureImage2d("Screenshots/One2ZeroColor" + std::to_string(i) + ".png", One2ZeroColor);
        Graphics::Get()->CaptureImage2d("Screenshots/Zero2OneDepth" + std::to_string(i) + ".png", Zero2OneDepth);
        Graphics::Get()->CaptureImage2d("Screenshots/One2ZeroDepth" + std::to_string(i) + ".png", One2ZeroDepth);

        CommandBuffer commandBuffer(true, VK_QUEUE_COMPUTE_BIT);

        // commandBuffer.Begin();
        descriptorSet_3.Push("UniformCamera", uniformCameraBlend);
        descriptorSet_3.Push("Zero2OneColor", Zero2OneColor);
        descriptorSet_3.Push("Zero2OneDepth", Zero2OneDepth);
        descriptorSet_3.Push("One2ZeroColor", One2ZeroColor);
        descriptorSet_3.Push("One2ZeroDepth", One2ZeroDepth);
        descriptorSet_3.Push("AlphaColor", AlphaColor);
        descriptorSet_3.Push("AlphaDepth", AlphaDepth);
        descriptorSet_3.Push("PushObject", pushHandlerWarpDepth);

        descriptorSet_3.Update(computeBlend);
        computeBlend.BindPipeline(commandBuffer);
        descriptorSet_3.BindDescriptor(commandBuffer, computeBlend);
        pushHandlerWarpDepth.BindPush(commandBuffer, computeBlend);
        computeBlend.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

        AlphaColor->Image2dPipelineBarrierComputeToCompute(commandBuffer);
        AlphaDepth->Image2dPipelineBarrierComputeToCompute(commandBuffer);

        commandBuffer.SubmitIdle();

        Graphics::Get()->CaptureImage2d("Screenshots/AlphaColor" + std::to_string(i) + ".png", AlphaColor);
        Graphics::Get()->CaptureImage2d("Screenshots/AlphaDepth" + std::to_string(i) + ".png", AlphaDepth);
    }
}

}   // namespace MapleLeaf