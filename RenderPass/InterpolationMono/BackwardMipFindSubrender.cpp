#include "BackwardMipFindSubrender.hpp"

#include "CommandBuffer.hpp"
#include "DescriptorHandler.hpp"
#include "Graphics.hpp"
#include "PipelineCompute.hpp"
#include "Scenes.hpp"

namespace MONO_Subrender {

BackwardMipFindSubrender::BackwardMipFindSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipelineBackwardFind("Shader/InterpolationMono/BackwardMipFind.comp", {}, false)
    , pipelineBackwardBlend("Shader/InterpolationMono/BackwardMipBlend.comp", {}, false)
    , uniformCameraBackwardFind(true)
    , pushHandlerBackwardFind(true)
    , uniformCameraBackwardBlend(true)
{}


void BackwardMipFindSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    const auto& prevMotionVector = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("prevMotionVector"));
    const auto& motionVector     = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("motionVector"));
    const auto& depth            = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("depth"));
    const auto& prevDepth        = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("prevDepth"));

    const auto& MultiLevelLighting     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelLighting"));
    const auto& MultiLevelPrevLighting = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelPrevLighting"));

    const auto& BackwardDepthAlpha2Zero    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardDepthAlpha2Zero"));
    const auto& BackwardDepthAlpha2One     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardDepthAlpha2One"));
    const auto& BackwardMipColorAlpha2Zero = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardMipColorAlpha2Zero"));
    const auto& BackwardMipColorAlpha2One  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardMipColorAlpha2One"));

    BackwardDepthAlpha2Zero->ClearImage2d(commandBuffer, glm::vec4(0.0f));
    BackwardDepthAlpha2One->ClearImage2d(commandBuffer, glm::vec4(0.0f));
    BackwardMipColorAlpha2Zero->ClearImage2d(commandBuffer, glm::vec4(0.0f));
    BackwardMipColorAlpha2One->ClearImage2d(commandBuffer, glm::vec4(0.0f));

    pushHandlerBackwardFind.Push("alpha", static_cast<float>(10) / static_cast<float>(30));
    pushHandlerBackwardFind.Push("mipLevel", mipLevel);

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCameraBackwardFind);

    descriptorSetBackwardFind.Push("UniformCamera", uniformCameraBackwardFind);
    descriptorSetBackwardFind.Push("PushObject", pushHandlerBackwardFind);

    descriptorSetBackwardFind.Push("prevMotionVector", prevMotionVector);
    descriptorSetBackwardFind.Push("motionVector", motionVector);
    descriptorSetBackwardFind.Push("prevDepth", prevDepth);
    descriptorSetBackwardFind.Push("depth", depth);

    descriptorSetBackwardFind.Push("MultiLevelPrevLighting", MultiLevelPrevLighting);
    descriptorSetBackwardFind.Push("MultiLevelLighting", MultiLevelLighting);

    //descriptorSetBackwardFind.Push("BackwardDepthAlpha2Zero", BackwardDepthAlpha2Zero, static_cast<uint32_t>(0), std::nullopt, std::nullopt);
    //descriptorSetBackwardFind.Push("BackwardDepthAlpha2One", BackwardDepthAlpha2One, static_cast<uint32_t>(0), std::nullopt, std::nullopt);
    descriptorSetBackwardFind.Push("BackwardMipColorAlpha2Zero", BackwardMipColorAlpha2Zero, mipLevel, std::nullopt, std::nullopt);
    descriptorSetBackwardFind.Push("BackwardMipColorAlpha2One", BackwardMipColorAlpha2One, mipLevel, std::nullopt, std::nullopt);

    if (!descriptorSetBackwardFind.Update(pipelineBackwardFind)) return;

    pipelineBackwardFind.BindPipeline(commandBuffer);

    descriptorSetBackwardFind.BindDescriptor(commandBuffer, pipelineBackwardFind);
    pushHandlerBackwardFind.BindPush(commandBuffer, pipelineBackwardFind);

    pipelineBackwardFind.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize() / uint32_t(1 << mipLevel));
}

void BackwardMipFindSubrender::Render(const CommandBuffer& commandBuffer) {}

void BackwardMipFindSubrender::PostRender(const CommandBuffer& commandBuffer)
{
    // const auto& BackwardDepthAlpha2Zero    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardDepthAlpha2Zero"));
    // const auto& BackwardDepthAlpha2One     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardDepthAlpha2One"));
    // const auto& BackwardAccurateAlpha2Zero = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardAccurateAlpha2Zero"));
    // const auto& BackwardAccurateAlpha2One  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardAccurateAlpha2One"));
    // const auto& AlphaColor                 = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaColor"));
    // const auto& AlphaDepth                 = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaDepth"));

    // auto camera = Scenes::Get()->GetScene()->GetCamera();
    // camera->PushUniforms(uniformCameraBackwardBlend);

    // descriptorSetBackwardBlend.Push("UniformCamera", uniformCameraBackwardBlend);
    // descriptorSetBackwardBlend.Push("BackwardAlpha2ZeroDepth", BackwardDepthAlpha2Zero);
    // descriptorSetBackwardBlend.Push("BackwardAlpha2OneDepth", BackwardDepthAlpha2One);
    // descriptorSetBackwardBlend.Push("BackwardAlpha2ZeroColor", BackwardAccurateAlpha2Zero);
    // descriptorSetBackwardBlend.Push("BackwardAlpha2OneColor", BackwardAccurateAlpha2One);
    // descriptorSetBackwardBlend.Push("PushObject", pushHandlerBackwardFind);

    // descriptorSetBackwardBlend.Push("AlphaColor", AlphaColor);
    // descriptorSetBackwardBlend.Push("AlphaDepth", AlphaDepth);

    // if (!descriptorSetBackwardBlend.Update(pipelineBackwardBlend)) return;

    // pipelineBackwardBlend.BindPipeline(commandBuffer);

    // descriptorSetBackwardBlend.BindDescriptor(commandBuffer, pipelineBackwardBlend);
    // pushHandlerBackwardFind.BindPush(commandBuffer, pipelineBackwardBlend);

    // pipelineBackwardBlend.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize() / uint32_t(1 << mipLevel));

    // if (frameID == 8) {
    //     ComputeBackward();
    //     exit(0);
    // }
}

void BackwardMipFindSubrender::ComputeBackward()
{
    PipelineCompute computeBackwardFind("Shader/InterpolationMono/BackwardFind.comp");
    PipelineCompute computeBackwardBlend("Shader/InterpolationMono/BackwardBlend.comp");

    const auto& prevMotionVector = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("prevMotionVector"));
    const auto& motionVector     = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("motionVector"));
    const auto& prevLighting     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("PrevLighting"));
    const auto& lighting         = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("lighting"));
    const auto& depth            = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("depth"));
    const auto& prevDepth        = dynamic_cast<const ImageDepth*>(Graphics::Get()->GetAttachment("prevDepth"));

    const auto& BlockAlpha2Zero            = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BlockAlpha2Zero"));
    const auto& BlockAlpha2One             = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BlockAlpha2One"));
    const auto& BackwardDepthAlpha2Zero    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardDepthAlpha2Zero"));
    const auto& BackwardDepthAlpha2One     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardDepthAlpha2One"));
    const auto& BackwardAccurateAlpha2Zero = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardAccurateAlpha2Zero"));
    const auto& BackwardAccurateAlpha2One  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardAccurateAlpha2One"));
    const auto& AlphaColor                 = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaColor"));
    const auto& AlphaDepth                 = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("AlphaDepth"));

    auto camera = Scenes::Get()->GetScene()->GetCamera();

    DescriptorsHandler descriptorSet(computeBackwardFind);
    DescriptorsHandler descriptorSet_2(computeBackwardBlend);

    for (int i = 0; i <= 30; i++) {
        pushHandlerBackwardFind.Push("alpha", static_cast<float>(i) / static_cast<float>(30));

        CommandBuffer commandBuffer(true, VK_QUEUE_COMPUTE_BIT);

        BlockAlpha2Zero->ClearImage2d(commandBuffer, glm::vec4(0.0f));
        BlockAlpha2One->ClearImage2d(commandBuffer, glm::vec4(0.0f));
        BackwardDepthAlpha2Zero->ClearImage2d(commandBuffer, glm::vec4(0.0f));
        BackwardDepthAlpha2One->ClearImage2d(commandBuffer, glm::vec4(0.0f));
        BackwardAccurateAlpha2Zero->ClearImage2d(commandBuffer, glm::vec4(0.0f));
        BackwardAccurateAlpha2One->ClearImage2d(commandBuffer, glm::vec4(0.0f));

        descriptorSet.Push("UniformCamera", uniformCameraBackwardFind);
        descriptorSet.Push("prevMotionVector", prevMotionVector);
        descriptorSet.Push("motionVector", motionVector);
        descriptorSet.Push("prevLighting", prevLighting);
        descriptorSet.Push("lighting", lighting);
        descriptorSet.Push("prevDepth", prevDepth);
        descriptorSet.Push("depth", depth);

        descriptorSet.Push("BlockAlpha2Zero", BlockAlpha2Zero);
        descriptorSet.Push("BlockAlpha2One", BlockAlpha2One);
        descriptorSet.Push("BackwardDepthAlpha2Zero", BackwardDepthAlpha2Zero);
        descriptorSet.Push("BackwardDepthAlpha2One", BackwardDepthAlpha2One);
        descriptorSet.Push("BackwardAccurateAlpha2Zero", BackwardAccurateAlpha2Zero);
        descriptorSet.Push("BackwardAccurateAlpha2One", BackwardAccurateAlpha2One);
        descriptorSet.Push("PushObject", pushHandlerBackwardFind);

        descriptorSet.Update(computeBackwardFind);
        computeBackwardFind.BindPipeline(commandBuffer);
        descriptorSet.BindDescriptor(commandBuffer, computeBackwardFind);
        pushHandlerBackwardFind.BindPush(commandBuffer, computeBackwardFind);

        computeBackwardFind.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

        BlockAlpha2One->Image2dPipelineBarrierComputeToCompute(commandBuffer);
        BlockAlpha2Zero->Image2dPipelineBarrierComputeToCompute(commandBuffer);
        BackwardDepthAlpha2Zero->Image2dPipelineBarrierComputeToCompute(commandBuffer);
        BackwardDepthAlpha2One->Image2dPipelineBarrierComputeToCompute(commandBuffer);
        BackwardAccurateAlpha2Zero->Image2dPipelineBarrierComputeToCompute(commandBuffer);
        BackwardAccurateAlpha2One->Image2dPipelineBarrierComputeToCompute(commandBuffer);

        commandBuffer.SubmitIdle();

        CommandBuffer commandBuffer_2(true, VK_QUEUE_COMPUTE_BIT);

        descriptorSet_2.Push("UniformCamera", uniformCameraBackwardBlend);
        descriptorSet_2.Push("BackwardAlpha2ZeroDepth", BackwardDepthAlpha2Zero);
        descriptorSet_2.Push("BackwardAlpha2OneDepth", BackwardDepthAlpha2One);
        descriptorSet_2.Push("BackwardAlpha2ZeroColor", BackwardAccurateAlpha2Zero);
        descriptorSet_2.Push("BackwardAlpha2OneColor", BackwardAccurateAlpha2One);
        descriptorSet_2.Push("PushObject", pushHandlerBackwardFind);

        descriptorSet_2.Push("AlphaColor", AlphaColor);
        descriptorSet_2.Push("AlphaDepth", AlphaDepth);

        descriptorSet_2.Update(computeBackwardBlend);
        computeBackwardBlend.BindPipeline(commandBuffer_2);
        descriptorSet_2.BindDescriptor(commandBuffer_2, computeBackwardBlend);
        pushHandlerBackwardFind.BindPush(commandBuffer_2, computeBackwardBlend);

        computeBackwardBlend.CmdRender(commandBuffer_2, Graphics::Get()->GetNonRTAttachmentSize());

        AlphaColor->Image2dPipelineBarrierComputeToCompute(commandBuffer_2);
        AlphaDepth->Image2dPipelineBarrierComputeToCompute(commandBuffer_2);

        commandBuffer_2.SubmitIdle();

        // Graphics::Get()->CaptureImage2d("Screenshots/BlockAlpha2Zero" + std::to_string(i) + ".png", BlockAlpha2Zero);
        // Graphics::Get()->CaptureImage2d("Screenshots/BlockAlpha2One" + std::to_string(i) + ".png", BlockAlpha2One);
        // Graphics::Get()->CaptureImage2d("Screenshots/BackwardDepthAlpha2Zero" + std::to_string(i) + ".png", BackwardDepthAlpha2Zero);
        // // Graphics::Get()->CaptureImage2d("Screenshots/BackwardDepthAlpha2One" + std::to_string(i) + ".png", BackwardDepthAlpha2One);

        Graphics::Get()->CaptureImage2d("Screenshots/BackwardAccurateAlpha2Zero" + std::to_string(i) + ".png", BackwardAccurateAlpha2Zero);
        Graphics::Get()->CaptureImage2d("Screenshots/BackwardAccurateAlpha2One" + std::to_string(i) + ".png", BackwardAccurateAlpha2One);
        Graphics::Get()->CaptureImage2d("Screenshots/AlphaColor" + std::to_string(i) + ".png", AlphaColor);
        Graphics::Get()->CaptureImage2d("Screenshots/AlphaDepth" + std::to_string(i) + ".png", AlphaDepth);
    }
}
}   // namespace MONO_Subrender