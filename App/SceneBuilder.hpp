#pragma once

#include "AssimpImporter.hpp"
#include "Camera.hpp"
#include "DefaultBuilder.hpp"
#include "DefaultMaterial.hpp"
#include "Scene.hpp"
#include "Scenes.hpp"
#include "ShadowRender.hpp"
#include "ShadowSystem.hpp"
#include "Skybox.hpp"
#include "Transform.hpp"

namespace MapleLeaf {
class SceneBuilder : public Scene
{
public:
    SceneBuilder(const std::filesystem::path path = "F:/SC-SSR-Scene/Wine5/Wine.gltf")
        : Scene()
        , path(path)
    {
        AddSystem<ShadowSystem>();
    }

    void Start() override
    {
        assimpImporter.Import(path, builder);

        auto shadows = Scenes::Get()->GetScene()->GetSystem<ShadowSystem>();
        for (const auto& node : builder.sceneGraph) {
            auto entity = CreateEntity();
            entity->SetName(node.name);
            entity->AddComponent(std::move(std::make_unique<Transform>(*node.transform)));
            if (!node.meshes.empty()) {
                for (int i = 0; i < node.meshes.size(); i++) {
                    // for every instance, instances use the same model, but maybe use different mat
                    entity->AddComponent<Mesh>(builder.meshes[node.meshes[i]]->GetModel(), builder.meshes[node.meshes[i]]->GetMaterial());
                    this->SetExtents(builder.meshes[node.meshes[i]]->GetModel()->GetMaxExtents(),
                                     builder.meshes[node.meshes[i]]->GetModel()->GetMinExtents(),
                                     node.transform->GetWorldMatrix());
                    if (shadows) entity->AddComponent<ShadowRender>();
                    Resources::Get()->Add(builder.meshes[node.meshes[i]]->GetModel());
                }
            }
        }

        // Create Skybox entity
        auto skyboxEntity = CreateEntity();
        skyboxEntity->SetName("skybox");
        skyboxEntity->AddComponent<Transform>();
        skyboxEntity->AddComponent<Skybox>("SkyboxClouds", false);

        if (builder.cameras.empty()) Log::Error("No camera can't rendering!");

        for (auto& camera : builder.cameras) {
            auto entity = Scenes::Get()->GetScene()->GetEntity(camera->GetName());
            entity->AddComponent(std::move(camera));
        }

        SetCamera(Scenes::Get()->GetScene()->GetComponent<Camera>());

        for (auto& light : builder.lights) {
            auto entity = Scenes::Get()->GetScene()->GetEntity(light->GetName());
            if (entity) {
                auto transform = entity->GetComponent<Transform>();

                glm::vec4 realDirection = transform->GetWorldMatrix() * glm::vec4(light->GetDirection(), 0.0f);

                light->SetPosition(transform->GetPosition());
                light->SetDirection(glm::vec3(realDirection));

                if (shadows && (light->type == LightType::Directional)) shadows->SetLightDirection(light->GetDirection());

                entity->AddComponent(std::move(light));
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