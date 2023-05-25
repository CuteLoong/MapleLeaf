#pragma once

#include "Camera.hpp"
#include "Engine.hpp"
#include "Graphics.hpp"
#include "Image2d.hpp"
#include "Log.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Model.hpp"
#include "Scene.hpp"
#include <filesystem>
#include <memory>
#include <utility>


namespace MapleLeaf {
class SceneBuilder : public Scene
{
public:
    SceneBuilder()
        : Scene(std::make_unique<Camera>())
    {
        std::cout << "Create Scene!" << std::endl;
    }

    void Start() override
    {
        for (const auto& [modelName, model] : models) {
            auto mesh = CreateEntity();
            mesh->AddComponent<Mesh>(model, nullptr);
        }
    }

    void Update() override { Scene::Update(); }

    bool IsPaused() const override { return false; }

    Model* AddModel(const std::string name, std::shared_ptr<Model>&& model)
    {
        if (models.count(name) != 0) Log::Info(name, " mesh replicate, please use instance draw!");

        models[name] = std::move(model);
        return models[name].get();
    }

    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T*, Material*>>>
    T* AddMaterial(const uint32_t index, std::unique_ptr<T>&& material)
    {
        if (materials.count(index) != 0) Log::Info(index, " material has been added, here maybe some warning!");

        materials[index] = std::move(material);
        return static_cast<T*>(materials[index].get());
    }

    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T*, Material*>>>
    bool loadMaterialTexture(std::unique_ptr<T>& material, Material::TextureSlot textureType, const std::filesystem::path& path)
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
    std::unordered_map<uint32_t, std::unique_ptr<Material>> materials;
    // TODO: Light
};

class Scenes : public Module::Registrar<Scenes>
{
    inline static const bool Registered = Register(Stage::Normal, Requires<Graphics>());

public:
    Scenes();

    void Update() override;

    Scene* GetScene() const { return scene.get(); }
    void   SetScene(std::unique_ptr<Scene>&& scene) { this->scene = std::move(scene); }

private:
    std::unique_ptr<Scene> scene;
};
}   // namespace MapleLeaf