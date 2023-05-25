#include "Scenes.hpp"

namespace MapleLeaf {
Scenes::Scenes() {}

void Scenes::Update()
{
    if (!scene) return;

    if (!scene->started) {
        scene->Start();
        scene->started = true;
    }

    scene->Update();
}
}   // namespace MapleLeaf