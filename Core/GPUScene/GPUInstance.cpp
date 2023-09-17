#include "GPUInstance.hpp"

#include "DefaultMaterial.hpp"
#include "Log.hpp"
#include "Scenes.hpp"
#include "Transform.hpp"

namespace MapleLeaf {
std::unordered_map<std::shared_ptr<Model>, std::pair<uint32_t, uint32_t>> GPUInstance::modelOffset{};
std::vector<Vertex3D>                                                     GPUInstance::verticesArray{};
std::vector<uint32_t>                                                     GPUInstance::indicesArray{};

GPUInstance::GPUInstance(Mesh* mesh, uint32_t instanceID, uint32_t materialID)
    : mesh(mesh)
    , instanceID(instanceID)
    , materialID(materialID)
    , instanceStatus(Status::Normal)
{
    model        = mesh->GetModel();
    modelMatrix  = mesh->GetEntity()->GetComponent<Transform>()->GetWorldMatrix();
    AABBLocalMin = model->GetMinExtents();
    AABBLocalMax = model->GetMaxExtents();
    indexCount   = model->GetIndexCount();
    vertexCount  = model->GetVertexCount();

    if (!modelOffset.count(model)) {
        indexOffset  = indicesArray.size();
        vertexOffset = verticesArray.size();

        std::copy(model->GetIndices().begin(), model->GetIndices().end(), std::back_inserter(indicesArray));
        std::copy(model->GetVertices().begin(), model->GetVertices().end(), std::back_inserter(verticesArray));
        modelOffset.emplace(model, std::make_pair(indexOffset, vertexOffset));
    }
    else {
        indexOffset  = modelOffset[model].first;
        vertexOffset = modelOffset[model].second;
    }
}

void GPUInstance::Update()
{
    modelMatrix = mesh->GetEntity()->GetComponent<Transform>()->GetWorldMatrix();
    if (mesh->GetUpdateStatus() == Mesh::UpdateStatus::MeshAlter) {
        // Need to be optimal
        if (mesh->GetModel() != model) {
            model        = mesh->GetModel();
            indexOffset  = indicesArray.size();
            vertexOffset = verticesArray.size();

            std::copy(model->GetIndices().begin(), model->GetIndices().end(), indicesArray.end());
            std::copy(model->GetVertices().begin(), model->GetVertices().end(), verticesArray.end());
            modelOffset.emplace(model, std::make_pair(indexOffset, vertexOffset));
        }
        instanceStatus = Status::Changed;

        // MaterialId Update if material add? or delete
    }
}
}   // namespace MapleLeaf