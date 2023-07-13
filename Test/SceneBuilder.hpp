#pragma once

#include "AssimpImporter.hpp"
#include "DefaultBuilder.hpp"
#include "DefaultMaterial.hpp"
#include "Scene.hpp"
#include <cstddef>
#include <filesystem>
#include "TestCamera.hpp"


namespace MapleLeaf {
class SceneBuilder : public Scene
{
public:
    SceneBuilder(const std::filesystem::path path = "E:/MapleLeaf/Resources/Models/DefaultCube/DefaultCube.gltf")
        : Scene(std::make_unique<TestCamera>())
        , path(path)
    {
        std::cout << "Create Scene!" << std::endl;
    }

    void Start() override
    {
        assimpImporter.Import(path, builder);
        for (const auto& [name, model] : builder.models) {
            auto mesh = CreateEntity();
            mesh->AddComponent<Mesh>(model, std::make_shared<DefaultMaterial>());
        }
    }

    void Update() override { Scene::Update(); }

    bool IsPaused() const override { return false; }

private:
    std::filesystem::path path;

    AssimpImporter<DefaultMaterial> assimpImporter;
    Builder                         builder;
};
}   // namespace MapleLeaf