#pragma once
#include "Color.hpp"
#include "glm/glm.hpp"
#include <vector>

namespace MapleLeaf {
class GPUInstances
{
public:
    GPUInstances() = default;

    void Update();

private:
    struct GPUInstanceData
    {
        glm::mat4 model;
        glm::vec3 AABBLocalMin;
        uint32_t  instanceID;
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

    std::vector<GPUMaterialData> materialDatas;
    std::vector<GPUInstanceData> instanceDatas;
};
}   // namespace MapleLeaf