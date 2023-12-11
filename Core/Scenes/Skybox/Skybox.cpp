#include "Skybox.hpp"

#include "Scenes.hpp"

namespace MapleLeaf {
Skybox::Skybox(const std::string& filename, bool enableRotation)
    : enableRotation(enableRotation)
    , filename(filename)
{}

void Skybox::Start()
{
    skyboxCube = ImageCube::Create(filename, ".png");
    if (!skyboxCube) return;

    transform = this->GetEntity()->GetComponent<Transform>();
}

void Skybox::Update() {}
}   // namespace MapleLeaf