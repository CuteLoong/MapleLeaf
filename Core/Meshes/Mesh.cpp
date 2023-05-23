#include "Mesh.hpp"

#include "glm/glm.hpp"

namespace MapleLeaf {
Mesh::Mesh(std::shared_ptr<Model> model, std::unique_ptr<Material>&& material)
    : model(std::move(model))
    , material(std::move(material))
{}

bool Mesh::CmdRender(const CommandBuffer& commandBuffer, UniformHandler& uniformScene, const Pipeline::Stage& pipelineStage)
{
    if (!model || !material) return false;

    // Check if we are in the correct pipeline stage.
    auto materialPipeline = material->GetPipelineMaterial();
    if (!materialPipeline || materialPipeline->GetStage() != pipelineStage) return false;

    // Binds the material pipeline.
    if (!materialPipeline->BindPipeline(commandBuffer)) return false;

    const auto& pipeline = *materialPipeline->GetPipeline();

    // Updates descriptors.
    descriptorSet.Push("UniformScene", uniformScene);
    descriptorSet.Push("UniformObject", uniformObject);

    material->PushDescriptors(descriptorSet);

    if (!descriptorSet.Update(pipeline)) return false;

    // Draws the object.
    descriptorSet.BindDescriptor(commandBuffer, pipeline);
    return model->CmdRender(commandBuffer);
}

void Mesh::SetMaterial(std::unique_ptr<Material>&& material)
{
    this->material = std::move(material);
    this->material->CreatePipeline(GetVertexInput());
}

bool Mesh::operator<(const Mesh& rhs) const
{
    // auto camera = Scenes::Get()->GetScene()->GetCamera();

    // auto transform0 = GetEntity()->GetComponent<Transform>();
    // auto transform1 = rhs.GetEntity()->GetComponent<Transform>();

    // auto thisDistance2 = (camera->GetPosition() - transform0->GetPosition()).LengthSquared();
    // auto otherDistance2 = (camera->GetPosition() - transform1->GetPosition()).LengthSquared();

    // return thisDistance2 > otherDistance2;
}

bool Mesh::operator>(const Mesh& rhs) const
{
    return !operator<(rhs);
}
}   // namespace MapleLeaf