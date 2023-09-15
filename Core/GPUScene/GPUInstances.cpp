#include "GPUInstances.hpp"

#include "DefaultMaterial.hpp"
#include "Log.hpp"
#include "Scenes.hpp"
#include "Transform.hpp"

namespace MapleLeaf {

GPUInstances::GPUInstances(std::shared_ptr<GPUUpdateInfos> updateInfos)
    : updateInfos(updateInfos)
{}

void GPUInstances::Start()
{
    auto meshes = Scenes::Get()->GetScene()->GetComponents<Mesh>();

    // foreach mesh
    for (const auto& mesh : meshes) {
        const auto& model     = mesh->GetModel();
        const auto& transform = mesh->GetEntity()->GetComponent<Transform>();
        const auto& material  = mesh->GetMaterial();

        GPUInstanceData instanceData;
        GPUMaterialData materialData;

        int32_t exsitModelIndex = models.count(model) == 0 ? -1 : models[model].first;
        int32_t exsitMatIndex   = materials.count(material) == 0 ? -1 : materials[material].first;

        instanceData.model        = transform->GetWorldMatrix();
        instanceData.AABBLocalMin = model->GetMinExtents();
        instanceData.AABBLocalMax = model->GetMaxExtents();
        instanceData.modelID      = exsitModelIndex == -1 ? models.size() : exsitModelIndex;
        instanceData.materialID   = exsitMatIndex == -1 ? materialDatas.size() : exsitMatIndex;

        instanceDatas.emplace(mesh, instanceData);
        if (exsitModelIndex == -1) {
            models.emplace(model, std::make_pair(models.size(), 0));
            updateInfos->AddUpdateModels(model);
        }

        if (exsitMatIndex == -1) {
            if (const DefaultMaterial* defaultMaterial = dynamic_cast<const DefaultMaterial*>(material.get())) {
                materialData.baseColor = defaultMaterial->GetBaseDiffuse();
                materialData.roughness = defaultMaterial->GetRoughness();
                materialData.metalic   = defaultMaterial->GetMetallic();

                if (const auto& diffuseImage = defaultMaterial->GetImageDiffuse()) {
                    materialData.baseColorTex = images.count(diffuseImage) == 0 ? images.size() : images[diffuseImage].first;
                    if (images.count(diffuseImage) == 0) images.emplace(diffuseImage, std::make_pair(images.size(), 0));
                    images[diffuseImage].second++;
                }
                if (const auto& normalImage = defaultMaterial->GetImageNormal()) {
                    materialData.baseColorTex = images.count(normalImage) == 0 ? images.size() : images[normalImage].first;
                    if (images.count(normalImage) == 0) images.emplace(normalImage, std::make_pair(images.size(), 0));
                    images[normalImage].second++;
                }
                if (const auto& materialImage = defaultMaterial->GetImageMaterial()) {
                    materialData.baseColorTex = images.count(materialImage) == 0 ? images.size() : images[materialImage].first;
                    if (images.count(materialImage) == 0) images.emplace(materialImage, std::make_pair(images.size(), 0));
                    images[materialImage].second++;
                }
            }
            else {
                Log::Error("Material Type is not found");
            }

            materialDatas.push_back(materialData);
            materials.emplace(material, std::make_pair(materials.size(), 0));
        }

        models[model].second++;
        materials[material].second++;
    }
}

void GPUInstances::Update()
{
    auto meshes = Scenes::Get()->GetScene()->GetComponents<Mesh>();

    for (const auto& mesh : meshes) {
        const auto& transform = mesh->GetEntity()->GetComponent<Transform>();
        if (transform->GetUpdateStatus() == Transform::UpdateStatus::None && mesh->GetUpdateStatus() == Mesh::UpdateStatus::None) continue;

        GPUInstanceData instanceData;
        GPUMaterialData materialData;

        // Now just transform update.
        if (transform->GetUpdateStatus() == Transform::UpdateStatus::Transformation && mesh->GetUpdateStatus() == Mesh::UpdateStatus::None) {
            instanceDatas[mesh].model = transform->GetWorldMatrix();
        }
        updateInfos->AddUpdateMeshDatas(mesh);
    }
}
}   // namespace MapleLeaf