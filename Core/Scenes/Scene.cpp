#include "Scene.hpp"

namespace MapleLeaf {
Scene::Scene(std::unique_ptr<Camera>&& camera)
    : camera(std::move(camera))
{}

void Scene::Update()
{
    systems.ForEach([](auto typeId, auto system) {
        if (system->IsEnabled()) system->Update();
    });

    entities.Update();
    camera->Update();
}
}   // namespace MapleLeaf