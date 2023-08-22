#include "ShadowCascade.hpp"
#include "Scenes.hpp"

namespace MapleLeaf {
ShadowCascade::ShadowCascade() {}

void ShadowCascade::Update(const Camera& camera, const glm::vec3& lightDirection)
{
    this->lightDirection = lightDirection;

    minExtent = Scenes::Get()->GetScene()->GetMinExtents();
    maxExtent = Scenes::Get()->GetScene()->GetMaxExtents();

    UpdateLightViewMatrix();
    UpdateLightOrthoMatrix();
}

void ShadowCascade::UpdateLightViewMatrix()
{
    glm::vec3 center = (minExtent + maxExtent) / 2.0f;
    glm::vec3 lightEye = glm::vec3(center.x, center.y, maxExtent.z) - lightDirection;
    lightViewMatrix = glm::lookAt(lightEye, center, glm::vec3(0.0f, 1.0f, 0.0f));
}

void ShadowCascade::UpdateLightOrthoMatrix()
{
    lightOrthoMatrix = glm::ortho(minExtent.x, maxExtent.x, minExtent.y, maxExtent.y, minExtent.z, maxExtent.z);
}
}   // namespace MapleLeaf