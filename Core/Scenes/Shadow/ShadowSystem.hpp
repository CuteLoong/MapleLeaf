#pragma once

#include "ShadowCascade.hpp"
#include "System.hpp"
#include "glm/glm.hpp"


namespace MapleLeaf {
class ShadowSystem : public System
{
public:
    ShadowSystem();

    void Update() override;

    const glm::vec3& GetLightDirection() const { return lightDirection; }
    void             SetLightDirection(const glm::vec3& lightDirection) { this->lightDirection = lightDirection; }

    int32_t GetShadowPcf() const { return shadowPcf; }
    void    SetShadowPcf(int32_t shadowPcf) { this->shadowPcf = shadowPcf; }

    float GetShadowBias() const { return shadowBias; }
    void  SetShadowBias(float shadowBias) { this->shadowBias = shadowBias; }

    const ShadowCascade& GetShadowCascade() const { return shadowCascade; }

private:
    glm::vec3 lightDirection;

    int32_t shadowPcf;
    float   shadowBias;

    ShadowCascade shadowCascade;
};
}   // namespace MapleLeaf