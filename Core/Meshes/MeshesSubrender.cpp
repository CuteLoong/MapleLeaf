#include "MeshesSubrender.hpp"

#include "Mesh.hpp"
#include "Scenes.hpp"


namespace MapleLeaf {
MeshesSubrender::MeshesSubrender(const Pipeline::Stage& pipelineStage, Sort sort)
    : Subrender(pipelineStage)
    , sort(sort)
    , uniformScene(true)
{}

void MeshesSubrender::Render(const CommandBuffer& commandBuffer)
{
    auto camera = Scenes::Get()->GetScene()->GetCamera();
    uniformScene.Push("projection", camera->GetProjectionMatrix());
    uniformScene.Push("view", camera->GetViewMatrix());
    uniformScene.Push("cameraPos", camera->GetPosition());

    auto meshes = Scenes::Get()->GetScene()->GetComponents<Mesh>();
    if (sort == Sort::Front)
        std::sort(meshes.begin(), meshes.end(), std::greater<>());
    else if (sort == Sort::Back)
        std::sort(meshes.begin(), meshes.end(), std::less<>());

    for (const auto& mesh : meshes) mesh->CmdRender(commandBuffer, uniformScene, GetStage());
}
}   // namespace MapleLeaf