#pragma once

#include "Image2d.hpp"
#include "Mesh.hpp"
#include "SceneGraph.hpp"

namespace MapleLeaf {
class Builder
{
    friend class SceneBuilder;

public:
    Builder() = default;

    Mesh* GetMesh(const uint32_t index);

    NodeID AddSceneNode(SceneNode&& node);

    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T*, Material*>>>
    Mesh* AddMesh(std::shared_ptr<Model>&& model, std::shared_ptr<T>&& material)
    {
        meshes.push_back(std::make_unique<Mesh>(model, material));
        return meshes.back().get();
    }

    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T*, Material*>>>
    bool loadMaterialTexture(std::shared_ptr<T>& material, Material::TextureSlot textureType, const std::filesystem::path& path)
    {
        if (material == nullptr) return false;

        auto image = Image2d::Create(path);

        if (textureType == Material::TextureSlot::BaseColor)
            material->SetImageDiffuse(std::move(image));
        else if (textureType == Material::TextureSlot::Normal)
            material->SetImageNormal(std::move(image));
        else if (textureType == Material::TextureSlot::Material)
            material->SetImageMaterial(std::move(image));
        else {
            Log::Error("Material::TextureSlot is not exit!");
            return false;
        }

        return true;
    }

private:
    std::vector<std::unique_ptr<Mesh>> meshes;
    SceneGraph                         sceneGraph;
    // TODO: Light
};
}   // namespace MapleLeaf