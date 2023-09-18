#pragma once

#include "Material.hpp"

namespace MapleLeaf {
class GPUMaterial
{
    friend class GPUScene;

public:
    GPUMaterial() = default;
    explicit GPUMaterial(const std::shared_ptr<Material>& material);

    static std::optional<uint32_t> GetMaterialID(const std::shared_ptr<Material>& material);

private:
    std::shared_ptr<Material> material;

    Color    baseColor;
    float    metalic;
    float    roughness;
    int32_t baseColorTex;
    int32_t normalTex;
    int32_t materialTex;

    static std::vector<std::shared_ptr<Material>> materialArray;
    static std::vector<std::shared_ptr<Image2d>>  images;
};
}   // namespace MapleLeaf