#pragma once

#include "Image2d.hpp"
#include "Mesh.hpp"

namespace MapleLeaf {
class Builder
{
    friend class SceneBuilder;

public:
    Builder() = default;

    Model* AddModel(const std::string name, std::shared_ptr<Model>&& model);
    Model* GetModel(const std::string name);

    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T*, Material*>>>
    T* AddMaterial(const uint32_t index, std::shared_ptr<T>&& material)
    {
        if (materials.count(index) != 0) Log::Info(index, " material has been added, here maybe some warning!");

        materials[index] = std::move(material);
        return static_cast<T*>(materials[index].get());
    }

    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T*, Material*>>>
    T* GetMaterial(const uint32_t index)
    {
        if (auto it = materials.find(index); it != materials.end() && it->second) return static_cast<T*>(it->second.get());

        return nullptr;
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
    std::unordered_map<std::string, std::shared_ptr<Model>> models;
    std::unordered_map<uint32_t, std::shared_ptr<Material>> materials;
    // TODO: Light
};
}   // namespace MapleLeaf