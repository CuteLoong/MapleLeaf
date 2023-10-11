#include "IndirectDrawSubrender.hpp"

#include "Scenes.hpp"

namespace MapleLeaf {
IndirectDrawSubrender::IndirectDrawSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"Shader/GPUDriven/Default.vert", "Shader/GPUDriven/Default.frag"}, {Vertex3D::GetVertexInput()}, {},
               PipelineGraphics::Mode::MRT)
{}

void IndirectDrawSubrender::PreRender(const CommandBuffer& commandBuffer) {}

void IndirectDrawSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    if (!gpuScene) return;

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    uniformScene.Push("projection", camera->GetProjectionMatrix());
    uniformScene.Push("view", camera->GetViewMatrix());
    uniformScene.Push("cameraPos", camera->GetPosition());

    descriptorSet.Push("UniformScene", uniformScene);
    gpuScene->PushDescriptors(descriptorSet);

    if (!descriptorSet.Update(pipeline)) return;
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);

    gpuScene->cmdRender(commandBuffer);
}

void IndirectDrawSubrender::PostRender(const CommandBuffer& commandBuffer) {}
}   // namespace MapleLeaf