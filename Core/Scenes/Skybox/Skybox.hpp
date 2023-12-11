#pragma once

#include "Color.hpp"
#include "Component.hpp"
#include "ImageCube.hpp"
#include "Transform.hpp"

namespace MapleLeaf {
class Skybox : public Component::Registrar<Skybox>
{
    inline static const bool Registrered = Register("skybox");

public:
    explicit Skybox(const std::string& filename = "SkyboxClouds", bool enableRotation = false);

    void Start() override;
    void Update() override;

    std::shared_ptr<ImageCube> GetImageCube() const { return skyboxCube; }
    Transform*                 GetTransform() const { return transform; }

private:
    std::string filename;
    bool        enableRotation;

    std::shared_ptr<ImageCube> skyboxCube;
    Transform*                 transform;
};
}   // namespace MapleLeaf