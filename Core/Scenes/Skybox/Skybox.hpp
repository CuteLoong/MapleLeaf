#pragma once

#include "Color.hpp"
#include "Component.hpp"

namespace MapleLeaf {
class Skybox : public Component::Registrar<Skybox>
{
    inline static const bool Registered = Register("skybox");

public:
    explicit Skybox(bool enableRotation = false);

    void Start() override;
    void Update() override;

private:
    bool enableRotation;
};
}   // namespace MapleLeaf