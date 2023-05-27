#pragma once

#include "AssimpImporter.hpp"
#include "DefaultBuilder.hpp"
#include "DefaultMaterial.hpp"
#include "Scene.hpp"
#include <filesystem>


namespace MapleLeaf {
class SceneBuilder : public Scene
{
public:
    SceneBuilder(const std::filesystem::path path = "E:/MapleLeaf/Resources/Models/DefaultCube/DefaultCube.gltf")
        : Scene(std::make_unique<Camera>())
        , path(path)
    {
        std::cout << "Create Scene!" << std::endl;
    }

    void Start() override { assimpImporter.Import(path, builder); }

    void Update() override { Scene::Update(); }

    bool IsPaused() const override { return false; }

private:
    std::filesystem::path path;

    AssimpImporter<DefaultMaterial> assimpImporter;
    Builder                         builder;
};
}   // namespace MapleLeaf