#pragma once

#include "Material.hpp"
#include "Mesh.hpp"


namespace MapleLeaf {
class GPUUpdateInfos
{
public:
    GPUUpdateInfos() = default;

    const std::vector<Mesh*>                  GetUpdatedMeshDatas() const { return updatedMeshDatas; }
    const std::vector<std::shared_ptr<Model>> GetUpdatedModels() const { return updatedModels; }

    void AddUpdateMeshDatas(Mesh* updatedMesh) { updatedMeshDatas.push_back(updatedMesh); }
    void AddUpdateModels(std::shared_ptr<Model> updatedModel) { updatedModels.push_back(updatedModel); }

    void ClearMeshUpdates() { updatedMeshDatas.clear(); }
    void ClearModelUpdates() { updatedModels.clear(); }

private:
    std::vector<Mesh*>                  updatedMeshDatas;
    std::vector<std::shared_ptr<Model>> updatedModels;
};
}   // namespace MapleLeaf