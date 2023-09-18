#include "IndirectDrawSubrender.hpp"

#include "Mesh.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
IndirectDrawSubrender::IndirectDrawSubrender(const Pipeline::Stage& pipelineStage)
    : Subrender(pipelineStage)
    , pipeline(pipelineStage, {"F:/MapleLeaf/Resources/Shader/GPUDriven/Test.vert", "F:/MapleLeaf/Resources/Shader/GPUDriven/Test.frag"},
               {Vertex3D::GetVertexInput()}, {}, PipelineGraphics::Mode::MRT)
{}

void IndirectDrawSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto camera = Scenes::Get()->GetScene()->GetCamera();
    uniformScene.Push("projection", camera->GetProjectionMatrix());
    uniformScene.Push("view", camera->GetViewMatrix());
    uniformScene.Push("cameraPos", camera->GetPosition());

    auto gpuScene = Scenes::Get()->GetScene()->GetGpuScene();

    gpuScene->cmdRender(commandBuffer, uniformScene, pipeline);
}
}   // namespace MapleLeaf