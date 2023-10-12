#include "IndirectDrawSubrender.hpp"

#include "DescriptorHandler.hpp"
#include "PushHandler.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
IndirectDrawSubrender::IndirectDrawSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , compute("Shader/GPUDriven/Culling.comp")
    , pipeline(pipelineStage, {"Shader/GPUDriven/Default.vert", "Shader/GPUDriven/Default.frag"}, {Vertex3D::GetVertexInput()}, {},
               PipelineGraphics::Mode::MRT)
{}

void IndirectDrawSubrender::PreRender(const CommandBuffer& commandBuffer)
{
    const auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    if (!gpuScene) return;
    if (!gpuScene->GetIndirectBuffer()) return;

    auto camera = Scenes::Get()->GetScene()->GetCamera();

    uint32_t    instanceCount = gpuScene->GetInstanceCount();
    PushHandler pushHandler(*compute.GetShader()->GetUniformBlock("PushObject"));

    pushHandler.Push("projection", camera->GetProjectionMatrix());
    pushHandler.Push("view", camera->GetViewMatrix());
    pushHandler.Push("cameraPos", camera->GetPosition());
    pushHandler.Push("up", camera->GetUpVector());
    pushHandler.Push("forward", camera->GetForward());
    pushHandler.Push("right", camera->GetRight());
    pushHandler.Push("nearPlane", camera->GetNearPlane());
    pushHandler.Push("farPlane", camera->GetFarPlane());
    pushHandler.Push("fieldOfView", camera->GetFieldOfView());
    pushHandler.Push("aspectRatio", camera->GetAspectRatio());
    pushHandler.Push("instanceCount", instanceCount);

    descriptorSetCompute.Push("InstanceDatas", gpuScene->GetInstanceDatasHandler());
    descriptorSetCompute.Push("DrawCommandBuffer", gpuScene->GetIndirectBuffer());
    descriptorSetCompute.Push("PushObject", pushHandler);

    if (!descriptorSetCompute.Update(compute)) return;
    compute.BindPipeline(commandBuffer);
    descriptorSetCompute.BindDescriptor(commandBuffer, compute);
    pushHandler.BindPush(commandBuffer, compute);
    compute.CmdRender(commandBuffer, glm::uvec2(instanceCount, 1));

    VkBufferMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType                 = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    imageMemoryBarrier.srcAccessMask         = VK_ACCESS_SHADER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask         = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    imageMemoryBarrier.buffer                = gpuScene->GetIndirectBuffer()->GetBuffer();
    imageMemoryBarrier.offset                = 0;
    imageMemoryBarrier.size                  = VK_WHOLE_SIZE;
    vkCmdPipelineBarrier(
        commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, 0, 0, nullptr, 1, &imageMemoryBarrier, 0, nullptr);
}

void IndirectDrawSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    if (!gpuScene) return;

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    uniformScene.Push("projection", camera->GetProjectionMatrix());
    uniformScene.Push("view", camera->GetViewMatrix());
    uniformScene.Push("cameraPos", camera->GetPosition());

    descriptorSetGraphics.Push("UniformScene", uniformScene);
    gpuScene->PushDescriptors(descriptorSetGraphics);

    if (!descriptorSetGraphics.Update(pipeline)) return;
    pipeline.BindPipeline(commandBuffer);

    descriptorSetGraphics.BindDescriptor(commandBuffer, pipeline);

    gpuScene->cmdRender(commandBuffer);
}

void IndirectDrawSubrender::PostRender(const CommandBuffer& commandBuffer) {}
}   // namespace MapleLeaf