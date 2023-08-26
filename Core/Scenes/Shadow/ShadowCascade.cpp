#include "ShadowCascade.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
ShadowCascade::ShadowCascade() {}

void ShadowCascade::Update(const Camera& camera, const glm::vec3& lightDirection)
{
    this->lightDirection = lightDirection;

    minExtents = Scenes::Get()->GetScene()->GetMinExtents();
    maxExtents = Scenes::Get()->GetScene()->GetMaxExtents();

    UpdateLightViewMatrix();
    UpdateLightOrthoMatrix();
}

void ShadowCascade::UpdateLightViewMatrix()
{
    glm::vec3 center = (minExtents + maxExtents) / 2.0f;
    float radius = glm::distance(minExtents, maxExtents) / 2.0;
    glm::vec3 lightEye = center - lightDirection * radius;

    lightViewMatrix = glm::lookAt(lightEye, lightEye + lightDirection, glm::vec3(0.0f, 1.0f, 0.0f));
}

void ShadowCascade::UpdateLightOrthoMatrix()
{
    //calc extent after use view matrix
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::infinity());
    glm::vec3 max = glm::vec3(-std::numeric_limits<float>::infinity());
    for (int i = 0; i < 8; i++) {
        glm::vec3 w = glm::vec3(i & 1, (i >> 1) & 1, (i >> 2) & 1);
        glm::vec4 v = lightViewMatrix * glm::vec4(glm::mix(minExtents, maxExtents, w), 1.0);

        min = glm::vec3(std::min(min.x, v.x), std::min(min.y, v.y), std::min(min.z, v.z));
        max = glm::vec3(std::max(max.x, v.x), std::max(max.y, v.y), std::max(max.z, v.z));
    }
    minExtents = min;
    maxExtents = max;

    float radius = glm::distance(minExtents, maxExtents) / 2.0;

    lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, std::min(abs(minExtents.z), abs(maxExtents.z)), std::max(abs(minExtents.z), abs(maxExtents.z)));
    // lightOrthoMatrix = glm::perspective(glm::radians(60.0f), Devices::Get()->GetWindow()->GetAspectRatio(), 0.1f, 100.0f);
}
}   // namespace MapleLeaf