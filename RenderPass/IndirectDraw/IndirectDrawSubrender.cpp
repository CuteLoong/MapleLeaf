#include "IndirectDrawSubrender.hpp"

#include "Scenes.hpp"

namespace MapleLeaf {
IndirectDrawSubrender::IndirectDrawSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"F:/MapleLeaf/Resources/Shader/GPUDriven/Test.vert", "F:/MapleLeaf/Resources/Shader/GPUDriven/Test.frag"},
               {Vertex3D::GetVertexInput()}, {}, PipelineGraphics::Mode::MRT)
{}

void IndirectDrawSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    if (!gpuScene) return;

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    uniformScene.Push("projection", camera->GetProjectionMatrix());
    uniformScene.Push("view", camera->GetViewMatrix());
    uniformScene.Push("cameraPos", camera->GetPosition());

    DrawCulling = Resources::Get()->GetThreadPool().Enqueue(
        ComputeFrustumCulling, uniformScene.GetUniformBuffer(), gpuScene->GetInstanceDatasHandler().GetStorageBuffer());

    descriptorSet.Push("UniformScene", uniformScene);
    descriptorSet.Push("DrawCommandBuffer", *DrawCulling);
    gpuScene->PushDescriptors(descriptorSet);

    if (!descriptorSet.Update(pipeline)) return;
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);

    gpuScene->cmdRender(commandBuffer, *DrawCulling);
}

std::unique_ptr<IndirectBuffer> IndirectDrawSubrender::ComputeFrustumCulling(const UniformBuffer* uniformSceneBuffer,
                                                                             const StorageBuffer* instanceBuffer)
{
    if (!uniformSceneBuffer || !instanceBuffer) return nullptr;
    auto     gpuScene      = Scenes::Get()->GetScene()->GetGpuScene();
    uint32_t instanceCount = gpuScene->GetInstanceCount();

    std::unique_ptr<IndirectBuffer> indirectBuffer = std::make_unique<IndirectBuffer>(instanceCount * sizeof(VkDrawIndexedIndirectCommand));

    CommandBuffer   commandBuffer(true, VK_QUEUE_COMPUTE_BIT);
    PipelineCompute compute("F:/MapleLeaf/Resources/Shader/GPUDriven/Culling.comp");

    compute.BindPipeline(commandBuffer);

    DescriptorsHandler descriptorSet(compute);
    PushHandler        pushHandler(*compute.GetShader()->GetUniformBlock("PushObject"));
    pushHandler.Push("instanceCount", instanceCount);

    descriptorSet.Push("InstanceDatas", instanceBuffer);
    descriptorSet.Push("DrawCommandBuffer", indirectBuffer);
    descriptorSet.Push("UniformScene", uniformSceneBuffer);
    descriptorSet.Push("PushObject", pushHandler);
    descriptorSet.Update(compute);

    descriptorSet.BindDescriptor(commandBuffer, compute);
    pushHandler.BindPush(commandBuffer, compute);
    compute.CmdRender(commandBuffer, glm::uvec2(instanceCount, 1));
    commandBuffer.SubmitIdle();

    return indirectBuffer;
}
}   // namespace MapleLeaf