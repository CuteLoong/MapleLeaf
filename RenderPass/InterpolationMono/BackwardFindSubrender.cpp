#include "BackwardFindSubrender.hpp"
#include "InterpolationMono/BackwardFindSubrender.hpp"
#include "Scenes.hpp"

namespace MONO_Subrender {
BackwardFindSubrender::BackwardFindSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineBackwardFind("Shader/InterpolationMono/BackwardFind.comp", {}, false)
    , uniformCameraBackwardFind(true)
    , pushHandlerBackwardFind(true)
{}

void BackwardFindSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    const auto& prevMotionVector = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("prevMotionVector"));
    const auto& motionVector     = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("motionVector"));
    const auto& prevLighting     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("PrevLighting"));
    const auto& lighting         = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("lighting"));

    const auto& BlockAlpha2Zero    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BlockAlpha2Zero"));
    const auto& BlockAlpha2One     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BlockAlpha2One"));
    const auto& BackwardAlpha2Zero = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardAlpha2Zero"));
    const auto& BackwardAlpha2One  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardAlpha2One"));

    BlockAlpha2Zero->ClearImage2d(commandBuffer, glm::vec4(0.0f));
    BlockAlpha2One->ClearImage2d(commandBuffer, glm::vec4(0.0f));
    BackwardAlpha2Zero->ClearImage2d(commandBuffer, glm::vec4(0.0f));
    BackwardAlpha2One->ClearImage2d(commandBuffer, glm::vec4(0.0f));

    pushHandlerBackwardFind.Push("alpha", static_cast<float>(0.7));

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCameraBackwardFind);

    descriptorSetBackwardFind.Push("UniformCamera", uniformCameraBackwardFind);
    descriptorSetBackwardFind.Push("PushObject", pushHandlerBackwardFind);
    descriptorSetBackwardFind.Push("prevMotionVector", prevMotionVector);
    descriptorSetBackwardFind.Push("motionVector", motionVector);
    descriptorSetBackwardFind.Push("prevLighting", prevLighting);
    descriptorSetBackwardFind.Push("lighting", lighting);

    descriptorSetBackwardFind.Push("BlockAlpha2Zero", BlockAlpha2Zero);
    descriptorSetBackwardFind.Push("BlockAlpha2One", BlockAlpha2One);
    descriptorSetBackwardFind.Push("BackwardAlpha2Zero", BackwardAlpha2Zero);
    descriptorSetBackwardFind.Push("BackwardAlpha2One", BackwardAlpha2One);

    if (!descriptorSetBackwardFind.Update(pipelineBackwardFind)) return;

    pipelineBackwardFind.BindPipeline(commandBuffer);

    descriptorSetBackwardFind.BindDescriptor(commandBuffer, pipelineBackwardFind);
    pushHandlerBackwardFind.BindPush(commandBuffer, pipelineBackwardFind);

    pipelineBackwardFind.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());
}

void BackwardFindSubrender::Render(const CommandBuffer& commandBuffer)
{
    frameID++;
}

void BackwardFindSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    if (frameID == 9) {
        ComputeBackward();
        exit(0);
    }
}

void BackwardFindSubrender::ComputeBackward()
{
    PipelineCompute computeBackwardFind("Shader/InterpolationMono/BackwardFind.comp");

    const auto& prevMotionVector = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("prevMotionVector"));
    const auto& motionVector     = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("motionVector"));
    const auto& prevLighting     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("PrevLighting"));
    const auto& lighting         = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("lighting"));

    const auto& BlockAlpha2Zero    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BlockAlpha2Zero"));
    const auto& BlockAlpha2One     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BlockAlpha2One"));
    const auto& BackwardAlpha2Zero = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardAlpha2Zero"));
    const auto& BackwardAlpha2One  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardAlpha2One"));

    auto camera = Scenes::Get()->GetScene()->GetCamera();

    DescriptorsHandler descriptorSet(computeBackwardFind);

    for (int i = 0; i <= 30; i++) {
        pushHandlerBackwardFind.Push("alpha", static_cast<float>(i) / static_cast<float>(30));

        CommandBuffer commandBuffer(true, VK_QUEUE_COMPUTE_BIT);

        BlockAlpha2Zero->ClearImage2d(commandBuffer, glm::vec4(0.0f));
        BlockAlpha2One->ClearImage2d(commandBuffer, glm::vec4(0.0f));

        descriptorSet.Push("UniformCamera", uniformCameraBackwardFind);
        descriptorSet.Push("prevMotionVector", prevMotionVector);
        descriptorSet.Push("motionVector", motionVector);
        descriptorSet.Push("prevLighting", prevLighting);
        descriptorSet.Push("lighting", lighting);
        descriptorSet.Push("BlockAlpha2Zero", BlockAlpha2Zero);
        descriptorSet.Push("BlockAlpha2One", BlockAlpha2One);
        descriptorSet.Push("BackwardAlpha2Zero", BackwardAlpha2Zero);
        descriptorSet.Push("BackwardAlpha2One", BackwardAlpha2One);
        descriptorSet.Push("PushObject", pushHandlerBackwardFind);

        descriptorSet.Update(computeBackwardFind);
        computeBackwardFind.BindPipeline(commandBuffer);
        descriptorSet.BindDescriptor(commandBuffer, computeBackwardFind);
        pushHandlerBackwardFind.BindPush(commandBuffer, computeBackwardFind);

        computeBackwardFind.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

        BlockAlpha2One->Image2dPipelineBarrierComputeToCompute(commandBuffer);
        BlockAlpha2Zero->Image2dPipelineBarrierComputeToCompute(commandBuffer);
        BackwardAlpha2Zero->Image2dPipelineBarrierComputeToCompute(commandBuffer);
        BackwardAlpha2One->Image2dPipelineBarrierComputeToCompute(commandBuffer);

        commandBuffer.SubmitIdle();

        Graphics::Get()->CaptureImage2d("Screenshots/BlockAlpha2Zero" + std::to_string(i) + ".png", BlockAlpha2Zero);
        Graphics::Get()->CaptureImage2d("Screenshots/BlockAlpha2One" + std::to_string(i) + ".png", BlockAlpha2One);
        Graphics::Get()->CaptureImage2d("Screenshots/BackwardAlpha2Zero" + std::to_string(i) + ".png", BackwardAlpha2Zero);
        Graphics::Get()->CaptureImage2d("Screenshots/BackwardAlpha2One" + std::to_string(i) + ".png", BackwardAlpha2One);
    }
}

}   // namespace MONO_Subrender