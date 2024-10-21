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

    const auto& MultiLevelLighting     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelLighting"));
    const auto& MultiLevelPrevLighting = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelPrevLighting"));
    const auto& MultiLevelDepth        = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelDepth"));
    const auto& MultiLevelPrevDepth    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelPrevDepth"));

    const auto& BackwardMipDepthAlpha2Zero = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardMipDepthAlpha2Zero"));
    const auto& BackwardMipDepthAlpha2One  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardMipDepthAlpha2One"));
    const auto& BackwardMipColorAlpha2Zero = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardMipColorAlpha2Zero"));
    const auto& BackwardMipColorAlpha2One  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardMipColorAlpha2One"));

    BackwardMipDepthAlpha2Zero->ClearImage2d(commandBuffer, glm::vec4(0.0f));
    BackwardMipDepthAlpha2One->ClearImage2d(commandBuffer, glm::vec4(0.0f));
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
    descriptorSetBackwardFind.Push("MultiLevelPrevDepth", MultiLevelPrevDepth);
    descriptorSetBackwardFind.Push("MultiLevelDepth", MultiLevelDepth);

    descriptorSetBackwardFind.Push("MultiLevelPrevLighting", MultiLevelPrevLighting);
    descriptorSetBackwardFind.Push("MultiLevelLighting", MultiLevelLighting);

    descriptorSetBackwardFind.Push("BackwardMipDepthAlpha2Zero", BackwardMipDepthAlpha2Zero, mipLevel, std::nullopt, std::nullopt);
    descriptorSetBackwardFind.Push("BackwardMipDepthAlpha2One", BackwardMipDepthAlpha2One, mipLevel, std::nullopt, std::nullopt);
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
    const auto& BackwardMipDepthAlpha2Zero = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardMipDepthAlpha2Zero"));
    const auto& BackwardMipDepthAlpha2One  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardMipDepthAlpha2One"));
    const auto& BackwardMipColorAlpha2Zero = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardMipColorAlpha2Zero"));
    const auto& BackwardMipColorAlpha2One  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardMipColorAlpha2One"));
    const auto& MipAlphaColor              = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MipAlphaColor"));
    const auto& MipAlphaDepth              = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MipAlphaDepth"));

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    camera->PushUniforms(uniformCameraBackwardBlend);

    descriptorSetBackwardBlend.Push("UniformCamera", uniformCameraBackwardBlend);
    descriptorSetBackwardBlend.Push("BackwardMipDepthAlpha2Zero", BackwardMipDepthAlpha2Zero);
    descriptorSetBackwardBlend.Push("BackwardMipDepthAlpha2One", BackwardMipDepthAlpha2One);
    descriptorSetBackwardBlend.Push("BackwardMipColorAlpha2Zero", BackwardMipColorAlpha2Zero);
    descriptorSetBackwardBlend.Push("BackwardMipColorAlpha2One", BackwardMipColorAlpha2One);
    descriptorSetBackwardBlend.Push("PushObject", pushHandlerBackwardFind);

    descriptorSetBackwardBlend.Push("MipAlphaColor", MipAlphaColor, mipLevel, std::nullopt, std::nullopt);
    descriptorSetBackwardBlend.Push("MipAlphaDepth", MipAlphaDepth, mipLevel, std::nullopt, std::nullopt);

    if (!descriptorSetBackwardBlend.Update(pipelineBackwardBlend)) return;

    pipelineBackwardBlend.BindPipeline(commandBuffer);

    descriptorSetBackwardBlend.BindDescriptor(commandBuffer, pipelineBackwardBlend);
    pushHandlerBackwardFind.BindPush(commandBuffer, pipelineBackwardBlend);

    pipelineBackwardBlend.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize() / uint32_t(1 << mipLevel));

    if (camera->frameID == 10) {
        ComputeBackward();
        exit(0);
    }
}

void BackwardMipFindSubrender::ComputeBackward()
{
    PipelineCompute computeBackwardFind("Shader/InterpolationMono/BackwardMipFind.comp");
    PipelineCompute computeBackwardBlend("Shader/InterpolationMono/BackwardMipBlend.comp");

    const auto& prevMotionVector = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("prevMotionVector"));
    const auto& motionVector     = dynamic_cast<const Image2d*>(Graphics::Get()->GetAttachment("motionVector"));

    const auto& MultiLevelLighting     = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelLighting"));
    const auto& MultiLevelPrevLighting = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelPrevLighting"));
    const auto& MultiLevelDepth        = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelDepth"));
    const auto& MultiLevelPrevDepth    = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MultiLevelPrevDepth"));

    const auto& BackwardMipDepthAlpha2Zero = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardMipDepthAlpha2Zero"));
    const auto& BackwardMipDepthAlpha2One  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardMipDepthAlpha2One"));
    const auto& BackwardMipColorAlpha2Zero = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardMipColorAlpha2Zero"));
    const auto& BackwardMipColorAlpha2One  = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("BackwardMipColorAlpha2One"));

    const auto& MipAlphaColor = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MipAlphaColor"));
    const auto& MipAlphaDepth = dynamic_cast<const Image2d*>(Graphics::Get()->GetNonRTAttachment("MipAlphaDepth"));

    auto camera = Scenes::Get()->GetScene()->GetCamera();

    DescriptorsHandler descriptorSet(computeBackwardFind);
    DescriptorsHandler descriptorSet_2(computeBackwardBlend);

    for (int i = 0; i <= 30; i++) {
        pushHandlerBackwardFind.Push("alpha", static_cast<float>(i) / static_cast<float>(30));

        CommandBuffer commandBuffer(true, VK_QUEUE_COMPUTE_BIT);

        BackwardMipDepthAlpha2Zero->ClearImage2d(commandBuffer, glm::vec4(0.0f));
        BackwardMipDepthAlpha2One->ClearImage2d(commandBuffer, glm::vec4(0.0f));
        BackwardMipColorAlpha2Zero->ClearImage2d(commandBuffer, glm::vec4(0.0f));
        BackwardMipColorAlpha2One->ClearImage2d(commandBuffer, glm::vec4(0.0f));

        descriptorSet.Push("UniformCamera", uniformCameraBackwardFind);
        descriptorSet.Push("prevMotionVector", prevMotionVector);
        descriptorSet.Push("motionVector", motionVector);
        descriptorSet.Push("MultiLevelPrevLighting", MultiLevelPrevLighting);
        descriptorSet.Push("MultiLevelLighting", MultiLevelLighting);
        descriptorSet.Push("MultiLevelPrevDepth", MultiLevelPrevDepth);
        descriptorSet.Push("MultiLevelDepth", MultiLevelDepth);

        descriptorSet.Push("BackwardMipDepthAlpha2Zero", BackwardMipDepthAlpha2Zero, mipLevel, std::nullopt, std::nullopt);
        descriptorSet.Push("BackwardMipDepthAlpha2One", BackwardMipDepthAlpha2One, mipLevel, std::nullopt, std::nullopt);
        descriptorSet.Push("BackwardMipColorAlpha2Zero", BackwardMipColorAlpha2Zero, mipLevel, std::nullopt, std::nullopt);
        descriptorSet.Push("BackwardMipColorAlpha2One", BackwardMipColorAlpha2One, mipLevel, std::nullopt, std::nullopt);
        descriptorSet.Push("PushObject", pushHandlerBackwardFind);

        descriptorSet.Update(computeBackwardFind);
        computeBackwardFind.BindPipeline(commandBuffer);
        descriptorSet.BindDescriptor(commandBuffer, computeBackwardFind);
        pushHandlerBackwardFind.BindPush(commandBuffer, computeBackwardFind);

        computeBackwardFind.CmdRender(commandBuffer, Graphics::Get()->GetNonRTAttachmentSize());

        BackwardMipDepthAlpha2Zero->Image2dPipelineBarrierComputeToCompute(commandBuffer, mipLevel);
        BackwardMipDepthAlpha2One->Image2dPipelineBarrierComputeToCompute(commandBuffer, mipLevel);
        BackwardMipColorAlpha2Zero->Image2dPipelineBarrierComputeToCompute(commandBuffer, mipLevel);
        BackwardMipColorAlpha2One->Image2dPipelineBarrierComputeToCompute(commandBuffer, mipLevel);

        commandBuffer.SubmitIdle();

        CommandBuffer commandBuffer_2(true, VK_QUEUE_COMPUTE_BIT);

        descriptorSet_2.Push("UniformCamera", uniformCameraBackwardBlend);
        descriptorSet_2.Push("BackwardMipDepthAlpha2Zero", BackwardMipDepthAlpha2Zero);
        descriptorSet_2.Push("BackwardMipDepthAlpha2One", BackwardMipDepthAlpha2One);
        descriptorSet_2.Push("BackwardMipColorAlpha2Zero", BackwardMipColorAlpha2Zero);
        descriptorSet_2.Push("BackwardMipColorAlpha2One", BackwardMipColorAlpha2One);
        descriptorSet_2.Push("PushObject", pushHandlerBackwardFind);

        descriptorSet_2.Push("MipAlphaColor", MipAlphaColor, mipLevel, std::nullopt, std::nullopt);
        descriptorSet_2.Push("MipAlphaDepth", MipAlphaDepth, mipLevel, std::nullopt, std::nullopt);

        descriptorSet_2.Update(computeBackwardBlend);
        computeBackwardBlend.BindPipeline(commandBuffer_2);
        descriptorSet_2.BindDescriptor(commandBuffer_2, computeBackwardBlend);
        pushHandlerBackwardFind.BindPush(commandBuffer_2, computeBackwardBlend);

        computeBackwardBlend.CmdRender(commandBuffer_2, Graphics::Get()->GetNonRTAttachmentSize());

        MipAlphaColor->Image2dPipelineBarrierComputeToCompute(commandBuffer_2);
        MipAlphaDepth->Image2dPipelineBarrierComputeToCompute(commandBuffer_2);

        commandBuffer_2.SubmitIdle();

        // Graphics::Get()->CaptureImage2d("Screenshots/BlockAlpha2Zero" + std::to_string(i) + ".png", BlockAlpha2Zero);
        // Graphics::Get()->CaptureImage2d("Screenshots/BlockAlpha2One" + std::to_string(i) + ".png", BlockAlpha2One);
        // Graphics::Get()->CaptureImage2d("Screenshots/BackwardDepthAlpha2Zero" + std::to_string(i) + ".png", BackwardDepthAlpha2Zero);
        // // Graphics::Get()->CaptureImage2d("Screenshots/BackwardDepthAlpha2One" + std::to_string(i) + ".png", BackwardDepthAlpha2One);

        Graphics::Get()->CaptureImage2d("Screenshots/BackwardAccurateAlpha2Zero" + std::to_string(i) + ".png", BackwardMipColorAlpha2Zero, mipLevel);
        Graphics::Get()->CaptureImage2d("Screenshots/BackwardAccurateAlpha2One" + std::to_string(i) + ".png", BackwardMipColorAlpha2One, mipLevel);
        Graphics::Get()->CaptureImage2d("Screenshots/MipAlphaColor" + std::to_string(i) + ".png", MipAlphaColor, mipLevel);
        Graphics::Get()->CaptureImage2d("Screenshots/MipAlphaDepth" + std::to_string(i) + ".png", MipAlphaDepth, mipLevel);
    }
}
}   // namespace MONO_Subrender