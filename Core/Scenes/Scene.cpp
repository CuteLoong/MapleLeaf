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

void Scene::SetExtents(const glm::vec3& maxExtent, const glm::vec3& minExtent, const glm::mat4& transfrom)
{
    for (int i = 0; i < 8; i++) {
        glm::vec3 w = glm::vec3(i & 1, (i >> 1) & 1, (i >> 2) & 1);
        glm::vec4 v = transfrom * glm::vec4(glm::mix(minExtent, maxExtent, w), 1.0);

        minExtents = glm::vec3(std::min(minExtents.x, v.x), std::min(minExtents.y, v.y), std::min(minExtents.z, v.z));
        maxExtents = glm::vec3(std::max(maxExtents.x, v.x), std::max(maxExtents.y, v.y), std::max(maxExtents.z, v.z));
    }
}
}   // namespace MapleLeaf