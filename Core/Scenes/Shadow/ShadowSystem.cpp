#include "ShadowSystem.hpp"

#include "Light.hpp"
#include "Scenes.hpp"


namespace MapleLeaf {
ShadowSystem::ShadowSystem()
    : lightDirection(0.0, 0.0, -1.0)
    , shadowBias(0.001f)
    , shadowPcf(1)
{}

void ShadowSystem::Update()
{
    if (auto camera = Scenes::Get()->GetScene()->GetCamera()) {
        shadowCascade.Update(*camera, lightDirection);
    }
}
}   // namespace MapleLeaf