#pragma once

#include "AssimpImporter.hpp"
#include "DefaultBuilder.hpp"
#include "DefaultMaterial.hpp"
#include "Scene.hpp"
#include "TestCamera.hpp"
#include "Transform.hpp"

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

        for (const auto& node : builder.sceneGraph) {
            auto entity = CreateEntity();
            entity->SetName(node.name);
            entity->AddComponent(std::move(std::make_unique<Transform>(*node.transform)));
            if (!node.meshes.empty()) {
                for (int i = 0; i < node.meshes.size(); i++) {
                    entity->AddComponent(std::move(builder.meshes[node.meshes[i]]));
                }
            }
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