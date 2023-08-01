#include "ShadowCascade.hpp"

namespace MapleLeaf {
ShadowCascade::ShadowCascade() {}

void ShadowCascade::Update(const Camera& camera, const glm::vec3& lightDirection) {
    this->lightDirection = lightDirection;

    UpdateLightViewMatrix();
    UpdateLightOrthoMatrix();
}
}   // namespace MapleLeaf