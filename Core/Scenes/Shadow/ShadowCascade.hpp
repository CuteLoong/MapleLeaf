#pragma once

#include "Camera.hpp"
#include "glm/glm.hpp"

namespace MapleLeaf {
class ShadowCascade
{
public:
    ShadowCascade();

    void Update(const Camera& camera, const glm::vec3& lightDirection);

    const glm::mat4 GetLightProjectionViewMatrix() const {return lightOrthoMatrix * lightViewMatrix;}

private:
    void UpdateLightViewMatrix();
    void UpdateLightOrthoMatrix();

    glm::vec3 lightDirection;

    glm::mat4 lightOrthoMatrix;
    glm::mat4 lightViewMatrix;
};
}   // namespace MapleLeaf

