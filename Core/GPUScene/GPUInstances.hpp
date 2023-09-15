#pragma once

#include "Color.hpp"
#include "GPUUpdateInfos.hpp"
#include "Image2d.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Model.hpp"

namespace MapleLeaf {
class GPUInstances
{
    friend class GPUScene;

public:
    // first is index of model/material, second is the model/material used count.
    using IndexCount = std::pair<uint32_t, uint32_t>;

    struct GPUInstanceData
    {
        glm::mat4 model;
        glm::vec3 AABBLocalMin;
        uint32_t  modelID;
        glm::vec3 AABBLocalMax;
        uint32_t  materialID;
    };

    struct GPUMaterialData
    {
        Color    baseColor;
        float    metalic;
        float    roughness;
        uint32_t baseColorTex;
        uint32_t normalTex;
        uint32_t materialTex;
    };

    explicit GPUInstances(std::shared_ptr<GPUUpdateInfos> updateInfos);

    void Start();
    void Update();

private:
    std::vector<GPUMaterialData>               materialDatas;
    std::unordered_map<Mesh*, GPUInstanceData> instanceDatas;

    std::unordered_map<std::shared_ptr<Model>, IndexCount>    models;
    std::unordered_map<std::shared_ptr<Material>, IndexCount> materials;
    std::unordered_map<std::shared_ptr<Image2d>, IndexCount>  images;   // include baseColorMap normalMap, materialMap(metalic, roughness)

    std::shared_ptr<GPUUpdateInfos> updateInfos;
};
}   // namespace MapleLeaf