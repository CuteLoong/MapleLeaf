#include "IndirectDrawStereoSubrender.hpp"

#include "Scenes.hpp"

namespace MapleLeaf {
IndirectDrawStereoSubrender::IndirectDrawStereoSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"Shader/GPUDriven/DefaultStereo.vert", "Shader/GPUDriven/Multiview.geom","Shader/GPUDriven/DefaultStereo.frag"},
               {Vertex3D::GetVertexInput()}, {}, PipelineGraphics::Mode::StereoMRT)
{}

void IndirectDrawStereoSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();
    if (!gpuScene) return;

    auto camera = Scenes::Get()->GetScene()->GetCamera();
    uniformScene.Push("projection", camera->GetStereoProjectionMatrix());
    uniformScene.Push("view", camera->GetStereoViewMatrix());
    uniformScene.Push("cameraPos", camera->GetPosition());

    descriptorSet.Push("UniformScene", uniformScene);
    gpuScene->PushDescriptors(descriptorSet);

    if (!descriptorSet.Update(pipeline)) return;
    pipeline.BindPipeline(commandBuffer);

    descriptorSet.BindDescriptor(commandBuffer, pipeline);

    gpuScene->cmdRender(commandBuffer);
}

void IndirectDrawStereoSubrender::PostRender(const CommandBuffer& commandBuffer) {}
}   // namespace MapleLeaf