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

    descriptorSet.Push("UniformScene", uniformScene);
    gpuScene->PushDescriptors(descriptorSet);

    if (!descriptorSet.Update(pipeline)) return;
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);

    gpuScene->cmdRender(commandBuffer);
}
}   // namespace MapleLeaf