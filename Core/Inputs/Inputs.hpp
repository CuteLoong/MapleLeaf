#pragma once

#include "Devices.hpp"
#include "Engine.hpp"
#include "Graphics.hpp"
#include "KeyEnums.hpp"
#include "Scenes.hpp"
#include "boost/signals2.hpp"

namespace MapleLeaf {

class Inputs : public Module::Registrar<Inputs>
{
    inline static const bool Registered = Register(Stage::Pre, Requires<Devices, Graphics, Scenes>());

public:
    Inputs();

    void Update() override;

    void             ResetPositionDelta() { positionDelta = glm::vec3(0.0f); }
    void             ResetRotationDelta() { rotationDelta = glm::vec2(0.0f); }
    const glm::vec3& GetPositionDelta() const { return positionDelta; }
    const glm::vec2& GetRotationDelta() const { return rotationDelta; }

private:
    glm::vec3 positionDelta;
    glm::vec2 rotationDelta;

    bool cusorLeftPress = false;

    void ProcessMouseButton(MouseButton mouseButton, InputAction inputAction, InputMod inputMod);
    void ProcessKeyboard(Key key, InputAction inputAction, InputMod inputMod);
    void ProcessMousePosition(glm::vec2 value);
};
}   // namespace MapleLeaf